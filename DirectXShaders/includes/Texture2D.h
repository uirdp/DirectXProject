#pragma once
#include "ComPtr.h"
#include <d3dx12.h>
#include <string>

class DescriptorHeap;
class DescriptorHandle;

class Texture2D
{
public:
	static Texture2D* Get(std::string path);
	static Texture2D* Get(std::wstring path);
	static Texture2D* Get(ID3D12Resource* buffer);
	static Texture2D* GetWhite();
	bool IsValid();

	ID3D12Resource* Resource();
	D3D12_SHADER_RESOURCE_VIEW_DESC ViewDesc();
	D3D12_SHADER_RESOURCE_VIEW_DESC ViewCubeMapDesc();

private:
	bool m_IsValid;
	std::wstring extension;
	Texture2D(std::string path);
	Texture2D(std::wstring path);
	Texture2D(ID3D12Resource* buffer);
	ComPtr<ID3D12Resource> m_pResource;
	bool Load(std::string& path);
	bool Load(std::wstring& path);

	static ID3D12Resource* GetDefaultResource(size_t width, size_t height);
	static ID3D12Resource* GetTextureCubeResource(size_t width, size_t height);

	Texture2D(const Texture2D&) = delete;
	void operator = (const Texture2D&) = delete;
};