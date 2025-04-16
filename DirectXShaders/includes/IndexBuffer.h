#pragma once
#include <cstdint>
#include <d3d12.h>
#include "ComPtr.h"

class IndexBuffer
{
public:

	IndexBuffer(size_t size, const void* pIndices = nullptr);
	D3D12_INDEX_BUFFER_VIEW View() const;
	bool IsValid();

	IndexBuffer(const IndexBuffer&) = delete;
	void operator = (const IndexBuffer&) = delete;

private:
	bool m_IsValid = false;
	ComPtr<ID3D12Resource> m_pBuffer = nullptr;
	D3D12_INDEX_BUFFER_VIEW m_View = {};
};