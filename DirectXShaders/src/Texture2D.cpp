#include "Texture2D.h"
#include <DirectXTex.h>
#include "Engine.h"

#pragma comment(lib, "DirectXTex.lib")

using namespace DirectX;

// TODO: AssimpLoaderと同じなので共通化する
std::wstring GetWideString(const std::string& str)
{
	auto num1 = MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED | MB_ERR_INVALID_CHARS, str.c_str(), -1, nullptr, 0);

	std::wstring wstr;
	wstr.resize(num1);

	auto num2 = MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED | MB_ERR_INVALID_CHARS, str.c_str(), -1, &wstr[0], num1);

	assert(num1 == num2);
	return wstr;
}

/// <summary>
///  
/// </summary>
/// <param name="filename"></param>
/// <returns> ファイルの拡張子 </returns>
std::wstring GetFileExtension(const std::wstring& filename)
{
	auto pos = filename.find_last_of(L".");
	if (pos == std::wstring::npos)
	{
		return L"";
	}
	return filename.substr(pos);
}

Texture2D::Texture2D(std::string path)
{
	m_IsValid = Load(path);
}

Texture2D::Texture2D(std::wstring path)
{
	m_IsValid = Load(path);
}

Texture2D::Texture2D(ID3D12Resource* buffer)
{
	m_pResource = buffer;
	m_IsValid = m_pResource != nullptr;
}

bool Texture2D::Load(std::string& path)
{
	auto wpath = GetWideString(path);
	return Load(wpath);
}

bool Texture2D::Load(std::wstring& path)
{
	TexMetadata metadata = {};
	ScratchImage scratchImg = {};
	auto ext = GetFileExtension(path);

	HRESULT hr = S_FALSE;
	if (ext == L".png")
	{
		hr = LoadFromWICFile(path.c_str(), WIC_FLAGS_NONE, &metadata, scratchImg);
	}
	else if (ext == L".tga")
	{
		hr = LoadFromTGAFile(path.c_str(), &metadata, scratchImg);
	}
	else if (ext == L".dds")
	{
		hr = LoadFromDDSFile(path.c_str(), DDS_FLAGS_NONE, &metadata, scratchImg);
	}
    else if (ext == L".hdr")
    {
		 hr = LoadFromHDRFile(path.c_str(), &metadata, scratchImg);
    }

	if (FAILED(hr))
	{
		printf("テクスチャの読み込みに失敗\n");
		return false;
	}

	extension = ext;

	auto img = scratchImg.GetImage(0, 0, 0);
	auto prop = CD3DX12_HEAP_PROPERTIES(D3D12_CPU_PAGE_PROPERTY_WRITE_BACK, D3D12_MEMORY_POOL_L0);
	auto desc = CD3DX12_RESOURCE_DESC::Tex2D(metadata.format, 
											 metadata.width, 
											 metadata.height, 
											 static_cast<UINT16>(metadata.arraySize),
											 static_cast<UINT16>(metadata.mipLevels));

	hr = g_Engine->Device()->CreateCommittedResource(
		&prop,
		D3D12_HEAP_FLAG_NONE,
		&desc,
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
		nullptr,
		IID_PPV_ARGS(m_pResource.ReleaseAndGetAddressOf())
	);

	if (FAILED(hr))
	{
		printf("テクスチャのリソース作成に失敗aa\n");
		return false;
	}

	if (ext == L".dds")
	{
		for (size_t item = 0; item < metadata.arraySize; ++item)
		{
			for (size_t mip = 0; mip < metadata.mipLevels; ++mip)
			{
				auto img = scratchImg.GetImage(mip, item, 0);
				UINT subresource = D3D12CalcSubresource(
					static_cast<UINT>(mip), static_cast<UINT>(item), 0,
					static_cast<UINT>(metadata.mipLevels), static_cast<UINT>(metadata.arraySize)
				);

				HRESULT hr2 = m_pResource->WriteToSubresource(
					subresource,
					nullptr,
					img->pixels,
					static_cast<UINT>(img->rowPitch),
					static_cast<UINT>(img->slicePitch)
				);
			}
		}
	}

	hr = m_pResource->WriteToSubresource(
		0,
		nullptr,
		img->pixels,
		static_cast<UINT>(img->rowPitch),
		static_cast<UINT>(img->slicePitch)
	);

	if (FAILED(hr))
	{
		printf("テクスチャのリソース書き込みに失敗\n");
		return false;
	}

	return true;
}

Texture2D* Texture2D::Get(std::string path)
{
	auto wpath = GetWideString(path);
	return Get(wpath);
}

Texture2D* Texture2D::Get(std::wstring path)
{
	auto tex = new Texture2D(path);
	if (!tex->IsValid())
	{
		return GetWhite();
	}
	return tex;
}

Texture2D* Texture2D::Get(ID3D12Resource* buffer)
{
	auto tex = new Texture2D(buffer);
	if (!tex->IsValid())
	{
		return GetWhite();
	}

	return tex;
}


Texture2D* Texture2D::GetWhite()
{
	ID3D12Resource* buff = GetDefaultResource(4, 4);

	std::vector<unsigned char> data(4 * 4 * 4);
	std::fill(data.begin(), data.end(), 0xff);

	auto hr = buff->WriteToSubresource(0, nullptr, data.data(), 4 * 4, data.size());
	if (FAILED(hr))
	{
		printf("テクスチャのリソース書き込みに失敗\n");
		return nullptr;
	}

	return new Texture2D(buff);
}

ID3D12Resource* Texture2D::GetDefaultResource(size_t width, size_t height)
{
	auto resDesc = CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_R8G8B8A8_UNORM, width, height);
	auto texHeapProp = CD3DX12_HEAP_PROPERTIES(D3D12_CPU_PAGE_PROPERTY_WRITE_BACK, D3D12_MEMORY_POOL_L0);

	ID3D12Resource* buff = nullptr;
	auto hr = g_Engine->Device()->CreateCommittedResource(
		&texHeapProp,
		D3D12_HEAP_FLAG_NONE,
		&resDesc,
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
		nullptr,
		IID_PPV_ARGS(&buff)
	);

	if (FAILED(hr))
	{
		printf("テクスチャのリソース作成に失敗\n");
		assert(SUCCEEDED(hr));
		return nullptr;
	}

	return buff;
}

bool Texture2D::IsValid()
{
	return m_IsValid;
}

ID3D12Resource* Texture2D::Resource()
{
	return m_pResource.Get();
}

D3D12_SHADER_RESOURCE_VIEW_DESC Texture2D::ViewDesc()
{
	D3D12_SHADER_RESOURCE_VIEW_DESC desc = {};
	desc.Format = m_pResource->GetDesc().Format;
	desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

	// キューブマップかどうか判定
	auto resDesc = m_pResource->GetDesc();
	if (resDesc.DepthOrArraySize % 6 == 0 && resDesc.DepthOrArraySize >= 6)
	{
		desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
		desc.TextureCube.MipLevels = 1;
	}
	else
	{
		desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		desc.Texture2D.MipLevels = 1;
	}
	return desc;
}

D3D12_SHADER_RESOURCE_VIEW_DESC Texture2D::ViewCubeMapDesc()
{
	D3D12_SHADER_RESOURCE_VIEW_DESC desc = {};
	desc.Format = m_pResource->GetDesc().Format;
	desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
	desc.TextureCube.MipLevels = 1;
	return desc;
}

