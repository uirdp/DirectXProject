#include "ConstantBuffer.h"
#include "Engine.h"

ConstantBuffer::ConstantBuffer(size_t size)
{
	size_t align = D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT;
	UINT64 sizeAligned = (size + (align - 1)) & ~(align - 1);

	auto prop = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	auto desc = CD3DX12_RESOURCE_DESC::Buffer(sizeAligned);

	auto hr = g_Engine->Device()->CreateCommittedResource(
		&prop,
		D3D12_HEAP_FLAG_NONE,
		&desc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(m_pBuffer.GetAddressOf())
	);

	if(FAILED(hr))
	{
		printf("定数バッファリソースの生成に失敗\n");
		return;
	}

	// 更新するのでMapしたまま
	hr = m_pBuffer->Map(0, nullptr, &m_pMappedPtr);
	if(FAILED(hr))
	{
		printf("定数バッファリソースのマップに失敗\n");
		return;
	}

	m_Desc = {};
	m_Desc.BufferLocation = m_pBuffer->GetGPUVirtualAddress();
	m_Desc.SizeInBytes = static_cast<UINT>(sizeAligned);

	m_IsValid = true;
}

bool ConstantBuffer::IsValid()
{
	return m_IsValid;
}

D3D12_GPU_VIRTUAL_ADDRESS ConstantBuffer::GetAddress() const
{
	return m_Desc.BufferLocation;
}

D3D12_CONSTANT_BUFFER_VIEW_DESC ConstantBuffer::ViewDesc() const
{
	return m_Desc;
}

// ここに値を入れればマップされる、アドレスの先頭から、サイズ<T>分だけ持ってくる
void* ConstantBuffer::GetPtr() const
{
	return m_pMappedPtr;
}