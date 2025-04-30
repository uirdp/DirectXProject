#include "Scene.h"
#include "Engine.h"
#include "App.h"
#include <d3dx12.h>
#include <vector>
#include "SharedStruct.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "ConstantBuffer.h"
#include "RootSignature.h"
#include "PipelineState.h"
#include "AssimpLoader.h"
#include "DescriptorHeap.h"
#include "Texture2D.h"
#include <iostream> // �f�o�b�O�p�ɒǉ�

Scene* g_Scene;

using namespace DirectX;

#include <filesystem>
namespace fs = std::filesystem;
std::wstring ReplaceExtension(const std::wstring& filename, const char* newExt)
{
	fs::path p = filename.c_str();
	return p.replace_extension(newExt).c_str();
}

VertexBuffer* vertexBuffer;
IndexBuffer* indexBuffer;
ConstantBuffer* constantBuffer[Engine::FRAME_BUFFER_COUNT];
ConstantBuffer* lightBuffer[Engine::FRAME_BUFFER_COUNT];
RootSignature* rootSignature;
PipelineState* pipelineState;
DescriptorHeap* descriptorHeap;
std::vector<DescriptorHandle*> materialHandles;

const wchar_t* modelFile = L"Assets/128ball.fbx";
std::vector<Mesh> meshes;
std::vector<VertexBuffer*> vertexBuffers;
std::vector<IndexBuffer*> indexBuffers;

bool Scene::Init()
{
	ImportSettings importSettings =
	{
		modelFile,
		meshes,
		false,
		true,
	};

	AssimpLoader loader;
	if (!loader.Load(importSettings))
	{
		printf("���f���̓ǂݍ��݂Ɏ��s\n");
		return false;
	}

	vertexBuffers.reserve(meshes.size()); // meshs�̃T�C�Y���������������m��
	for (size_t i = 0; i < meshes.size(); ++i)
	{
		auto vertexSize = sizeof(Vertex) * meshes[i].Vertices.size();
		auto vertexStride = sizeof(Vertex);
		auto vertices = meshes[i].Vertices.data();
		auto pVB = new VertexBuffer(vertexSize, vertexStride, vertices);
		if (!pVB->IsValid())
		{
			printf("���_�o�b�t�@�̐����Ɏ��s\n");
			return false;
		}

		vertexBuffers.push_back(pVB);
	}

	// mesh�̐������C���f�b�N�X�o�b�t�@���m��
	indexBuffers.reserve(meshes.size()); // meshs�̐��������������m��
	for (size_t i = 0; i < meshes.size(); ++i)
	{
		auto indexSize = sizeof(uint32_t) * meshes[i].Indices.size(); // ��̃��b�V���̃C���f�b�N�X�o�b�t�@�̃T�C�Y
		auto indices = meshes[i].Indices.data(); // ��̃��b�V���̃C���f�b�N�X�o�b�t�@�̃|�C���^
		auto pIB = new IndexBuffer(indexSize, indices); 
		if (!pIB->IsValid())
		{
			printf("�C���f�b�N�X�o�b�t�@�̐����Ɏ��s\n");
			return false;
		}
		indexBuffers.push_back(pIB);
	}
	SceneData sceneData = {};
	sceneData.Lights[0].Position = { 10.0f, 20.0f, 30.0f };
	sceneData.Lights[1].Position = { -10.0f, 20.0f, -30.0f };
	sceneData.Lights[2].Position = { 0.0f, 0.0f, 0.0f };
	sceneData.Lights[3].Position = { 3.0f, 0.0f, 0.0f };
	sceneData.LightCount = 2;

	for (size_t i = 0; i < Engine::FRAME_BUFFER_COUNT; i++)
	{
		lightBuffer[i] = new ConstantBuffer(sizeof(SceneData));
		if (!lightBuffer[i]->IsValid())
		{
			printf("���C�g�p�萔�o�b�t�@�̐����Ɏ��s\n");
			return false;
		}

		auto ptr = lightBuffer[i]->GetPtr<SceneData>();
		*ptr = sceneData;
	}


	auto eyePos = XMVectorSet(0.0f, 120.0f, 75.0f, 0.0f);
	auto targetPos = XMVectorSet(0.0f, 120.0, 0.0, 0.0f);
	auto upward = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	
	auto aspect = static_cast<float>(WINDOW_WIDTH) / static_cast<float>(WINDOW_HEIGHT);

	auto eyePos2 = XMFLOAT3(0.0f, 120.0f, 75.0f);
	auto upward2 = XMFLOAT3(0.0f, 1.0f, 0.0f);

	m_pCamera = new Camera(eyePos2, upward2, 90.0f, 0.0f);
	auto fov = XMConvertToRadians(m_pCamera->GetZoom());

	for (size_t i = 0; i < Engine::FRAME_BUFFER_COUNT; i++)
	{
		constantBuffer[i] = new ConstantBuffer(sizeof(Transform));
		if (!constantBuffer[i]->IsValid())
		{
			printf("�萔�o�b�t�@�̐����Ɏ��s\n");
			return false;
		}

		// map����Ă���
		auto ptr = constantBuffer[i]->GetPtr<Transform>();
		ptr->World = XMMatrixIdentity();
		ptr->View = m_pCamera->GetViewMatrix();
		ptr->Projection = XMMatrixPerspectiveFovRH(fov, aspect, 0.3f, 1000.0f);
		ptr->WorldInvTranspose = XMMatrixIdentity();
	}

	descriptorHeap = new DescriptorHeap();
	materialHandles.clear();
	for (size_t i = 0; i < meshes.size(); i++)
	{
		auto texPath = ReplaceExtension(meshes[i].DiffuseMapPath, ".tga");
		auto mainTex = Texture2D::Get(texPath);
		auto handle = descriptorHeap->Register(mainTex);
		materialHandles.push_back(handle);
	}

	rootSignature = new RootSignature();
	if (!rootSignature->IsValid())
	{
		printf("���[�g�V�O�l�`���̐����Ɏ��s\n");
		return false;
	}

	pipelineState = new PipelineState();
	pipelineState->SetInputLayout(Vertex::InputLayout);
	pipelineState->SetRootSignature(rootSignature->Get());

	if (IsDebuggerPresent())
	{
		pipelineState->SetVertexShader(L"../x64/Debug/SampleVS.cso");
		pipelineState->SetPixelShader(L"../x64/Debug/PBR.cso");
	}
	else
	{
		pipelineState->SetVertexShader(L"SampleVS.cso");
		pipelineState->SetPixelShader(L"PBR.cso");
	}

	pipelineState->Create();
	if (!pipelineState->IsValid())
	{
		printf("�p�C�v���C���X�e�[�g�̐����Ɏ��s\n");
		return false;
	}

	printf("�V�[���̏������ɐ���\n");
	return true;
}


float rotateY = 0.0f;
void Scene::Update()
{
	ProcessInput();

	rotateY += 0.02f;
	auto currentIndex = g_Engine->CurrentBackBufferIndex();
	auto currentTransform = constantBuffer[currentIndex]->GetPtr<Transform>();
	currentTransform->World = XMMatrixRotationY(rotateY);
	currentTransform->WorldInvTranspose = XMMatrixTranspose(XMMatrixInverse(nullptr, currentTransform->World));
	// View�s����f�o�b�O�p�ɏo��
	XMFLOAT4X4 viewMatrix;
	XMStoreFloat4x4(&viewMatrix, currentTransform->View);

	currentTransform->View = m_pCamera->GetViewMatrix();

	currentTransform->Projection = XMMatrixPerspectiveFovRH(XMConvertToRadians(m_pCamera->GetZoom()), 
		static_cast<float>(WINDOW_WIDTH) / static_cast<float>(WINDOW_HEIGHT), 0.3f, 1000.0f);
}

void Scene::Draw()
{
	auto currentIndex = g_Engine->CurrentBackBufferIndex();
	auto commandList = g_Engine->CommandList();
	auto materialHeap = descriptorHeap->Get();

	for (size_t i = 0; i < meshes.size(); i++)
	{
		auto vbView = vertexBuffers[i]->View();
		auto ibView = indexBuffers[i]->View();

		commandList->SetGraphicsRootSignature(rootSignature->Get());
		commandList->SetPipelineState(pipelineState->Get());
		// slot0�Ƀo�C���h�����
		commandList->SetGraphicsRootConstantBufferView(0, constantBuffer[currentIndex]->GetAddress());
	    commandList->SetGraphicsRootConstantBufferView(2, lightBuffer[currentIndex]->GetAddress());

		commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		commandList->IASetVertexBuffers(0, 1, &vbView);
		commandList->IASetIndexBuffer(&ibView);

		commandList->SetDescriptorHeaps(1, &materialHeap);
		commandList->SetGraphicsRootDescriptorTable(1, materialHandles[i]->HandleGPU);

		commandList->DrawIndexedInstanced(meshes[i].Indices.size(), 1, 0, 0, 0);
	}
	
}

void Scene::ProcessMouseMovement(int xOffset, int yOffset)
{
	static int lastX = WINDOW_WIDTH / 2;
	static int lastY = WINDOW_HEIGHT / 2;

	float xOffsetFloat = static_cast<float>(xOffset) ;
	float yOffsetFloat = static_cast<float>(yOffset);

	m_pCamera->ProcessMouseMovement(xOffset, yOffset);
}

void Scene::ProcessInput()
{
	// �J�����̈ړ�����
	if (g_KeyStates['W'] && g_KeyStates['A']) // ����Ɉړ�
	{
		m_pCamera->ProcessKeyboard(FORWARD_LEFT, g_DeltaTime);
	}
	else if (g_KeyStates['W'] && g_KeyStates['D']) // �E��Ɉړ�
	{
		m_pCamera->ProcessKeyboard(FORWARD_RIGHT, g_DeltaTime);
	}
	else if (g_KeyStates['S'] && g_KeyStates['A']) // �����Ɉړ�
	{
		m_pCamera->ProcessKeyboard(BACKWARD_LEFT, g_DeltaTime);
	}
	else if (g_KeyStates['S'] && g_KeyStates['D']) // �E���Ɉړ�
	{
		m_pCamera->ProcessKeyboard(BACKWARD_RIGHT, g_DeltaTime);
	}
	else
	{
		// �P������̈ړ�
		if (g_KeyStates['W'])
			m_pCamera->ProcessKeyboard(FORWARD, g_DeltaTime);
		if (g_KeyStates['S'])
			m_pCamera->ProcessKeyboard(BACKWARD, g_DeltaTime);
		if (g_KeyStates['A'])
			m_pCamera->ProcessKeyboard(LEFT, g_DeltaTime);
		if (g_KeyStates['D'])
			m_pCamera->ProcessKeyboard(RIGHT, g_DeltaTime);
	}
}

void Scene::UpdateCamera(CameraMovement movement, float deltaTime)
{
	m_pCamera->ProcessKeyboard(movement, deltaTime);
}


