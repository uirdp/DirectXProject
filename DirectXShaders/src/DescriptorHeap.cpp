#include "DescriptorHeap.h"
#include "Texture2D.h"
#include <d3dx12.h>
#include "Engine.h"

const UINT HANDLE_MAX = 512;

DescriptorHeap::DescriptorHeap()
{
	m_pHandles.clear();
	m_pHandles.reserve(HANDLE_MAX);

	D3D12_DESCRIPTOR_HEAP_DESC desc = {};
	desc.NodeMask = 1; // 0�ł����̂ł́H
	desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	desc.NumDescriptors = HANDLE_MAX;
	desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

	auto device = g_Engine->Device();

	auto hr = device->CreateDescriptorHeap(
		&desc, IID_PPV_ARGS(m_pHeap.ReleaseAndGetAddressOf()));

	if (FAILED(hr))
	{
		m_IsValid = false;
		return;
	}

	m_IncrementSize = device->GetDescriptorHandleIncrementSize(desc.Type);
	m_IsValid = true;
}

ID3D12DescriptorHeap* DescriptorHeap::Get() const
{
	return m_pHeap.Get();
}

DescriptorHandle* DescriptorHeap::Register(Texture2D* texture)
{
	auto count = m_pHandles.size();
	if (HANDLE_MAX <= count)
	{
		return nullptr;
	}

	DescriptorHandle* pHandle = new DescriptorHandle();

	auto handleCPU = m_pHeap->GetCPUDescriptorHandleForHeapStart(); //�@�q�[�v�̐擪�̃A�h���X
	handleCPU.ptr += m_IncrementSize * count; //�@�擪����count�ԖڂɃ��\�[�X��ǉ�

	auto handleGPU = m_pHeap->GetGPUDescriptorHandleForHeapStart();
	handleGPU.ptr += m_IncrementSize * count; //�@�擪����count�ԖڂɃ��\�[�X��ǉ�

	pHandle->HandleCPU = handleCPU;
	pHandle->HandleGPU = handleGPU;

	auto device = g_Engine->Device();
	auto resource = texture->Resource();
	auto desc = texture->ViewDesc();
	device->CreateShaderResourceView(resource, &desc, pHandle->HandleCPU);

	m_pHandles.push_back(pHandle);
	return pHandle;
}