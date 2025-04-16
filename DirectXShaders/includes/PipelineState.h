#pragma once
#include "ComPtr.h"
#include <d3dx12.h>
#include <string>

class PipelineState
{
public:
	PipelineState();
	bool IsValid();

	void SetInputLayout(D3D12_INPUT_LAYOUT_DESC layout);
	void SetRootSignature(ID3D12RootSignature* rootSignature);
	void SetVertexShader(std::wstring path);
	void SetPixelShader(std::wstring path);
	void Create();

	ID3D12PipelineState* Get();

private:
	bool m_IsValid = false;
	D3D12_GRAPHICS_PIPELINE_STATE_DESC desc = {};
	ComPtr<ID3D12PipelineState> m_pPipelineState;
	ComPtr<ID3DBlob> m_pVSBlob;
	ComPtr<ID3DBlob> m_pPSBlob;
};