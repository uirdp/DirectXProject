#pragma once
#include <d3d12.h>
#include "ComPtr.h"

class VertexBuffer
{
public:
	VertexBuffer(size_t size, size_t stride, const void* pInitData);
	D3D12_VERTEX_BUFFER_VIEW View() const;
	bool IsValid();

	// コピー禁止
	// 自身を引数にとるコンストラクタ（コピーコンストラクタ）を禁止
	VertexBuffer(const VertexBuffer&) = delete;
	// 代入演算子を禁止
	void operator = (const VertexBuffer&) = delete;

private:
	bool m_IsValid = false;
	ComPtr<ID3D12Resource> m_pBuffer = nullptr;
	D3D12_VERTEX_BUFFER_VIEW m_View = {};
};