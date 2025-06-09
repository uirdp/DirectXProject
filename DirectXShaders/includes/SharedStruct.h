#pragma once
#include <d3dx12.h>
#include <DirectXMath.h>
#include "ComPtr.h"

struct Vertex
{
public:
	DirectX::XMFLOAT3 Position;
	DirectX::XMFLOAT3 Normal;
	DirectX::XMFLOAT2 UV;
	DirectX::XMFLOAT3 Tangent;
	DirectX::XMFLOAT4 Color;
	static const D3D12_INPUT_LAYOUT_DESC InputLayout;

private:
	static const int InputElementCount = 5;
	static const D3D12_INPUT_ELEMENT_DESC InputElements[InputElementCount];
};

struct VertexPositionOnly
{
public:
	DirectX::XMFLOAT3 Position;
	static const D3D12_INPUT_LAYOUT_DESC InputLayout;
private:
	static const int InputElementCount = 1;
	static const D3D12_INPUT_ELEMENT_DESC InputElements[InputElementCount];
};

struct alignas(256) Transform
{
	DirectX::XMMATRIX World;
	DirectX::XMMATRIX View;
	DirectX::XMMATRIX Projection;
	DirectX::XMMATRIX WorldInvTranspose;
};

struct Mesh
{
	std::vector<Vertex> Vertices;
	std::vector<uint32_t> Indices;
	std::wstring DiffuseMapPath;
};

struct alignas(16) Light
{
	DirectX::XMFLOAT3 Position;
	float Intensity;
};

struct alignas(256) SceneData
{
	Light Lights[4];               // 16バイト * 4
	int LightCount;                // 4バイト
	DirectX::XMFLOAT3 CameraPosition;    // 12バイト
};


