#include "IndexBuffer.h"
#include <d3dx12.h>
#include "Engine.h"

IndexBuffer::IndexBuffer(size_t size, const void* pInitData)
{
	auto prop = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	D3D12_RESOURCE_DESC desc = CD3DX12_RESOURCE_DESC::Buffer(size);

	auto hr = g_Engine->Device()->CreateCommittedResource(
		&prop,
		D3D12_HEAP_FLAG_NONE,
		&desc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(m_pBuffer.GetAddressOf())
	);

	if (FAILED(hr))
	{
		printf("インデックスバッファリソースの生成に失敗\n");
		return;
	}

	m_View = {};
	m_View.BufferLocation = m_pBuffer->GetGPUVirtualAddress();
	m_View.Format = DXGI_FORMAT_R32_UINT;
	m_View.SizeInBytes = static_cast<UINT>(size);

	if(pInitData != nullptr)
	{
		void* p;
		hr = m_pBuffer->Map(0, nullptr, &p);

		if (FAILED(hr))
		{
			printf("インデックスバッファリソースのマップに失敗\n");
			return;
		}

		memcpy(p, pInitData, size);
		m_pBuffer->Unmap(0, nullptr);
	}

	m_IsValid = true;
}

bool IndexBuffer::IsValid()
{
	return m_IsValid;
}

D3D12_INDEX_BUFFER_VIEW IndexBuffer::View() const
{
	return m_View;
}