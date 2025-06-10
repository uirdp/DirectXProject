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
#include <iostream> // デバッグ用に追加

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
VertexBuffer* skyboxVertexBuffer;
IndexBuffer* skyboxIndexBuffer;
ConstantBuffer* constantBuffer[Engine::FRAME_BUFFER_COUNT];
ConstantBuffer* sceneBuffer[Engine::FRAME_BUFFER_COUNT];
ConstantBuffer* skyboxBuffer[Engine::FRAME_BUFFER_COUNT];
RootSignature* rootSignature;
PipelineState* pipelineState;
RootSignature* skyboxRootSignature;
PipelineState* skyboxPipelineState;
DescriptorHeap* descriptorHeap;
std::vector<DescriptorHandle*> materialHandles;
DescriptorHandle* skyboxHandle;
ComPtr<ID3D12Resource> IrradianceMap;
CD3DX12_CPU_DESCRIPTOR_HANDLE irradianceRtvHandle = {};
D3D12_GPU_DESCRIPTOR_HANDLE irradianceSrvHandle = {};

const wchar_t* modelFile = L"Assets/bunny.fbx";
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
		printf("モデルの読み込みに失敗\n");
		return false;
	}

	vertexBuffers.reserve(meshes.size()); // meshesのサイズ分だけメモリを確保
	for (size_t i = 0; i < meshes.size(); ++i)
	{
		auto vertexSize = sizeof(Vertex) * meshes[i].Vertices.size();
		auto vertexStride = sizeof(Vertex);
		auto vertices = meshes[i].Vertices.data();
		auto pVB = new VertexBuffer(vertexSize, vertexStride, vertices);
		if (!pVB->IsValid())
		{
			printf("頂点バッファの生成に失敗\n");
			return false;
		}

		vertexBuffers.push_back(pVB);
	}

	// meshの数だけインデックスバッファを確保
	indexBuffers.reserve(meshes.size()); // meshesの数だけメモリを確保
	for (size_t i = 0; i < meshes.size(); ++i)
	{
		auto indexSize = sizeof(uint32_t) * meshes[i].Indices.size(); // 一つのメッシュのインデックスバッファのサイズ
		auto indices = meshes[i].Indices.data(); // 一つのメッシュのインデックスバッファのポインタ
		auto pIB = new IndexBuffer(indexSize, indices); 
		if (!pIB->IsValid())
		{
			printf("インデックスバッファの生成に失敗\n");
			return false;
		}
		indexBuffers.push_back(pIB);
	}

	auto targetPos = XMVectorSet(0.0f, 120.0, 0.0, 0.0f);
	auto upward = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	
	auto aspect = static_cast<float>(WINDOW_WIDTH) / static_cast<float>(WINDOW_HEIGHT);

	auto eyePos = XMFLOAT3(0.0f, 120.0f, 75.0f);
	auto upward2 = XMFLOAT3(0.0f, 1.0f, 0.0f);

	m_pCamera = new Camera(eyePos, upward2, 0.0f, 0.0f);
	auto fov = XMConvertToRadians(m_pCamera->GetZoom());

	for (size_t i = 0; i < Engine::FRAME_BUFFER_COUNT; i++)
	{
		constantBuffer[i] = new ConstantBuffer(sizeof(Transform));
		if (!constantBuffer[i]->IsValid())
		{
			printf("定数バッファの生成に失敗\n");
			return false;
		}

		// mapされている
		auto ptr = constantBuffer[i]->GetPtr<Transform>();
        ptr->World = XMMatrixTranslation(0.0f, -60.0f, 0.0f) * XMMatrixRotationX(XMConvertToRadians(0.0f)) * XMMatrixScaling(2.0f, 2.0f, 2.0f);
		ptr->View = m_pCamera->GetViewMatrix();
		ptr->Projection = XMMatrixPerspectiveFovRH(fov, aspect, 0.3f, 1000.0f);
		ptr->WorldInvTranspose = XMMatrixIdentity();
	}

	SceneData sceneData = {};
	sceneData.Lights[0].Position = { 1000.0f, 1000.0f, 1000.0f };
	sceneData.Lights[1].Position = { -10.0f, 20.0f, -30.0f };
	sceneData.Lights[2].Position = { 0.0f, 0.0f, 0.0f };
	sceneData.Lights[3].Position = { 3.0f, 0.0f, 0.0f };
	sceneData.LightCount = 1;
	sceneData.CameraPosition = eyePos;

	for (size_t i = 0; i < Engine::FRAME_BUFFER_COUNT; i++)
	{
		sceneBuffer[i] = new ConstantBuffer(sizeof(SceneData));
		if (!sceneBuffer[i]->IsValid())
		{
			printf("ライト用定数バッファの生成に失敗\n");
			return false;
		}

		auto ptr = sceneBuffer[i]->GetPtr<SceneData>();
		*ptr = sceneData;
	}

	descriptorHeap = new DescriptorHeap();
	materialHandles.clear();
	for (size_t i = 0; i < meshes.size(); i++)
	{
		auto texPath = ReplaceExtension(meshes[i].DiffuseMapPath, ".tga");
	    texPath = meshes[i].DiffuseMapPath;
		auto mainTex = Texture2D::Get(texPath);
		auto handle = descriptorHeap->Register(mainTex);
		materialHandles.push_back(handle);
	}

      VertexPositionOnly skyboxVertices[] = {
		  {{-1.0f, -1.0f, -1.0f}}, // 0: 左下前
		  {{ 1.0f, -1.0f, -1.0f}}, // 1: 右下前
		  {{ 1.0f,  1.0f, -1.0f}}, // 2: 右上前
		  {{-1.0f,  1.0f, -1.0f}}, // 3: 左上前
		  {{-1.0f, -1.0f,  1.0f}}, // 4: 左下奥
		  {{ 1.0f, -1.0f,  1.0f}}, // 5: 右下奥
		  {{ 1.0f,  1.0f,  1.0f}}, // 6: 右上奥
		  {{-1.0f,  1.0f,  1.0f}}, // 7: 左上奥
        };

	// 面の向きを内向きに反転（時計回り → 反時計回り）
	uint32_t skyboxIndices[] = {
		// 前面
		0, 1, 2,  0, 2, 3,
		// 右面
		1, 5, 6,  1, 6, 2,
		// 奥面
		5, 4, 7,  5, 7, 6,
		// 左面
		4, 0, 3,  4, 3, 7,
		// 上面
		3, 2, 6,  3, 6, 7,
		// 下面
		4, 5, 1,  4, 1, 0
	};

	auto vertexSize = sizeof(VertexPositionOnly) * std::size(skyboxVertices);
	auto vertexStride = sizeof(VertexPositionOnly);
	skyboxVertexBuffer = new VertexBuffer(vertexSize, vertexStride, skyboxVertices);
	if (!skyboxVertexBuffer->IsValid())
	{
		printf("スカイボックス用頂点バッファの生成に失敗\n");
		return false;
	}

	auto indexSize = sizeof(uint32_t) * std::size(skyboxIndices);
	skyboxIndexBuffer = new IndexBuffer(indexSize, skyboxIndices);
	if (!skyboxIndexBuffer->IsValid())
	{
		printf("スカイボックス用インデックスバッファの生成に失敗\n");
		return false;
	}

	for (size_t i = 0; i < Engine::FRAME_BUFFER_COUNT; i++)
	{
		skyboxBuffer[i] = new ConstantBuffer(sizeof(Transform));
		if (!skyboxBuffer[i]->IsValid())
		{
			printf("定数バッファの生成に失敗\n");
			return false;
		}

		// mapされている
		auto ptr = skyboxBuffer[i]->GetPtr<Transform>();
		ptr->World = XMMatrixIdentity() * XMMatrixScaling(500.0f, 500.0f, 500.0f);
		ptr->View = m_pCamera->GetViewMatrix();
		ptr->Projection = XMMatrixPerspectiveFovRH(fov, aspect, 0.3f, 1000.0f);
		ptr->WorldInvTranspose = XMMatrixIdentity();
	}

	{
		auto texPath = L"Assets/Texture/BrightSky.dds";
		auto skyBox = Texture2D::Get(texPath);
		skyboxHandle = descriptorHeap->Register(skyBox);
	}
	

	rootSignature = new RootSignature();
	if (!rootSignature->IsValid())
	{
		printf("ルートシグネチャの生成に失敗\n");
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
		printf("パイプラインステートの生成に失敗\n");
		return false;
	}

	skyboxRootSignature = new RootSignature();
	if (!skyboxRootSignature->IsValid())
	{
		printf("スカイボックス用のルートシグネチャの生成に失敗\n");
	}

	skyboxPipelineState = new PipelineState();
	skyboxPipelineState->SetInputLayout(VertexPositionOnly::InputLayout);
	skyboxPipelineState->SetRootSignature(skyboxRootSignature->Get());

	if (IsDebuggerPresent())
	{
		skyboxPipelineState->SetVertexShader(L"../x64/Debug/SkyboxVS.cso");
		skyboxPipelineState->SetPixelShader(L"../x64/Debug/SkyboxPS.cso");
	}

	else
	{
		skyboxPipelineState->SetVertexShader(L"SkyboxVS.cso");
		skyboxPipelineState->SetPixelShader(L"SkyboxPS.cso");
	}

	skyboxPipelineState->Create();
	if (!skyboxPipelineState->IsValid())
	{
		printf("スカイボックス用パイプラインステートの生成に失敗");
	}

	printf("シーンの初期化に成功\n");
	return true;
}


float rotateY = 0.0f;
float rotateX = 90.0f;
void Scene::Update()
{
	ProcessInput();

	rotateY += 0.02f;
	auto currentIndex = g_Engine->CurrentBackBufferIndex();
	auto currentTransform = constantBuffer[currentIndex]->GetPtr<Transform>();
	// currentTransform->World = XMMatrixRotationY(rotateY);
	currentTransform->WorldInvTranspose = XMMatrixTranspose(XMMatrixInverse(nullptr, currentTransform->World));
	// View行列をデバッグ用に出力
	XMFLOAT4X4 viewMatrix;
	XMStoreFloat4x4(&viewMatrix, currentTransform->View);

	currentTransform->View = m_pCamera->GetViewMatrix();

	currentTransform->Projection = XMMatrixPerspectiveFovRH(XMConvertToRadians(m_pCamera->GetZoom()), 
		static_cast<float>(WINDOW_WIDTH) / static_cast<float>(WINDOW_HEIGHT), 0.3f, 1000.0f);

	auto currentScene = sceneBuffer[currentIndex]->GetPtr<SceneData>();
	currentScene->CameraPosition = m_pCamera->GetCameraPosition();

	auto currentSkybox = skyboxBuffer[currentIndex]->GetPtr<Transform>();
	currentSkybox->View = m_pCamera->GetViewMatrix();
	currentSkybox->View.r[3] = XMVectorSet(0, 0, 0, 1);
}

void Scene::Draw()
{
	auto currentIndex = g_Engine->CurrentBackBufferIndex();
	auto commandList = g_Engine->CommandList();
	auto materialHeap = descriptorHeap->Get();

	auto vbView = skyboxVertexBuffer->View();
	auto ibView = skyboxIndexBuffer->View();

	commandList->SetGraphicsRootSignature(skyboxRootSignature->Get());
	commandList->SetPipelineState(skyboxPipelineState->Get());

	commandList->SetGraphicsRootConstantBufferView(3, skyboxBuffer[currentIndex]->GetAddress());

	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	commandList->IASetVertexBuffers(0, 1, &vbView);
	commandList->IASetIndexBuffer(&ibView);

	commandList->SetDescriptorHeaps(1, &materialHeap);
	commandList->SetGraphicsRootDescriptorTable(1, skyboxHandle->HandleGPU);
	
	commandList->DrawIndexedInstanced(36, 1, 0, 0, 0);


	for (size_t i = 0; i < meshes.size(); i++)
	{
		auto vbView = vertexBuffers[i]->View();
		auto ibView = indexBuffers[i]->View();

		commandList->SetGraphicsRootSignature(rootSignature->Get());
		commandList->SetPipelineState(pipelineState->Get());
		// slot0にバインドされる
		commandList->SetGraphicsRootConstantBufferView(0, constantBuffer[currentIndex]->GetAddress());
	    commandList->SetGraphicsRootConstantBufferView(2, sceneBuffer[currentIndex]->GetAddress());

		commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		commandList->IASetVertexBuffers(0, 1, &vbView);
		commandList->IASetIndexBuffer(&ibView);

		commandList->SetDescriptorHeaps(1, &materialHeap);
		commandList->SetGraphicsRootDescriptorTable(1, materialHandles[i]->HandleGPU);

		commandList->DrawIndexedInstanced(meshes[i].Indices.size(), 1, 0, 0, 0);
	}
	
}

bool Scene::CreateIrradianceMapResource()
{
	auto heapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
	auto texDesc = CD3DX12_RESOURCE_DESC::Tex2D(
		DXGI_FORMAT_R16G16B16A16_FLOAT,
		32, 32, 6, 1, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET
	);
	
	D3D12_CLEAR_VALUE clearValue = {};
	clearValue.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
	clearValue.Color[0] = 0.0f;
	clearValue.Color[1] = 0.0f;
	clearValue.Color[2] = 0.0f;
	clearValue.Color[3] = 1.0f;

	HRESULT hr = g_Engine->Device()->CreateCommittedResource(
		&heapProp,
		D3D12_HEAP_FLAG_NONE,
		&texDesc,
		D3D12_RESOURCE_STATE_RENDER_TARGET,
		&clearValue,
		IID_PPV_ARGS(IrradianceMap.ReleaseAndGetAddressOf())
		);
	if (FAILED(hr))
	{
		printf("イラディアンスマップリソース作成失敗\n");
		return false;
	}

	D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
	rtvHeapDesc.NumDescriptors = 6;
	rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	ComPtr<ID3D12DescriptorHeap> rtvHeap;
	hr = g_Engine->Device()->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&rtvHeap));
	if (FAILED(hr)) return false;
	irradianceRtvHandle = rtvHeap->GetCPUDescriptorHandleForHeapStart();

	for (UINT i = 0; i < 6; ++i)
	{
		D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
		rtvDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
		rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DARRAY;
		rtvDesc.Texture2DArray.MipSlice = 0;
		rtvDesc.Texture2DArray.FirstArraySlice = i;
		rtvDesc.Texture2DArray.ArraySize = 1;
		rtvDesc.Texture2DArray.PlaneSlice = 0;
		g_Engine->Device()->CreateRenderTargetView(IrradianceMap.Get(), &rtvDesc, irradianceRtvHandle);
		irradianceRtvHandle.ptr += g_Engine->Device()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
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
	// カメラの移動処理
	if (g_KeyStates['W'] && g_KeyStates['A']) // 左上に移動
	{
		m_pCamera->ProcessKeyboard(FORWARD_LEFT, g_DeltaTime);
	}
	else if (g_KeyStates['W'] && g_KeyStates['D']) // 右上に移動
	{
		m_pCamera->ProcessKeyboard(FORWARD_RIGHT, g_DeltaTime);
	}
	else if (g_KeyStates['S'] && g_KeyStates['A']) // 左下に移動
	{
		m_pCamera->ProcessKeyboard(BACKWARD_LEFT, g_DeltaTime);
	}
	else if (g_KeyStates['S'] && g_KeyStates['D']) // 右下に移動
	{
		m_pCamera->ProcessKeyboard(BACKWARD_RIGHT, g_DeltaTime);
	}
	else
	{
		// 単一方向の移動
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


