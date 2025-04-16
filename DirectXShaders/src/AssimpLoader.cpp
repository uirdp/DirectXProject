#include "AssimpLoader.h"
#include "SharedStruct.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <d3dx12.h>
#include <filesystem>

namespace fs = std::filesystem;

std::wstring GetDirectoryPath(const std::wstring& path)
{
	fs::path p = path.c_str();
	return p.remove_filename().c_str();
}

std::string ToUTF8(const std::wstring& str)
{
	auto length = WideCharToMultiByte(CP_UTF8, 0U, str.data(), 
		-1, nullptr, 0, nullptr, nullptr);
	auto buffer = new char[length];

	WideCharToMultiByte(CP_UTF8, 0U, str.data(), -1, buffer, length, nullptr, nullptr);

	std::string result(buffer);
	delete[] buffer;
	buffer = nullptr;

	return result;
}

std::wstring ToWideString(const std::string& str) 
{
	auto num1 = MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED | MB_ERR_INVALID_CHARS,
									str.c_str(), -1, nullptr, 0);

	std::wstring wstr;
	wstr.resize(num1);

	auto num2 = MultiByteToWideChar(CP_UTF8, MB_PRECOMPOSED | MB_ERR_INVALID_CHARS,
		str.c_str(), -1, &wstr[0], num1);

	assert(num1 == num2);
	return wstr;
}

bool AssimpLoader::Load(ImportSettings settings)
{
    // ファイル名が指定されていない場合はfalseを返す
    if (settings.filename == nullptr)
    {
        return false;
    }

    auto& meshes = settings.meshes;
    auto flipU = settings.flipU;
    auto flipV = settings.flipV;

    // ファイルパスをUTF-8に変換
    auto path = ToUTF8(settings.filename);

    Assimp::Importer importer;
    int flag = 0;
    flag |= aiProcess_Triangulate;
    flag |= aiProcess_PreTransformVertices;
    flag |= aiProcess_CalcTangentSpace;
    flag |= aiProcess_GenSmoothNormals;
    flag |= aiProcess_GenUVCoords;
    flag |= aiProcess_RemoveRedundantMaterials;
    flag |= aiProcess_OptimizeMeshes;

    // ファイルを読み込む
    auto scene = importer.ReadFile(path, flag);

    // 読み込みに失敗した場合はエラーメッセージを表示してfalseを返す
    if (scene == nullptr)
    {
        printf("Error: %s\n", importer.GetErrorString());
        return false;
    }

    // Mesh構造体へ変換
    meshes.clear();
    meshes.resize(scene->mNumMeshes);

    for (size_t i = 0; i < meshes.size(); ++i)
    {
        const auto pMesh = scene->mMeshes[i];
        LoadMesh(meshes[i], pMesh, flipU, flipV);
        const auto pMaterial = scene->mMaterials[i];
        LoadTexture(settings.filename, meshes[i], pMaterial);
    }

    scene = nullptr;

    return true;
}

void AssimpLoader::LoadMesh(Mesh& dst, const aiMesh* src, bool flipU, bool flipV)
{
	aiVector3D zero3D(0.0f, 0.0f, 0.0f);
	aiColor4D zeroColor(0.0f, 0.0f, 0.0f, 0.0f);

	dst.Vertices.resize(src->mNumVertices);
    
    for (auto i = 0u; i < src->mNumVertices; ++i)
    {
        auto position = &(src->mVertices[i]);
		auto normal = &(src->mNormals[i]);
		auto uv = (src->HasTextureCoords(0)) ? &(src->mTextureCoords[0][i]) : &zero3D;
		auto tangent = (src->HasTangentsAndBitangents()) ? &(src->mTangents[i]) : &zero3D;
		auto color = (src->HasVertexColors(0)) ? &(src->mColors[0][i]) : &zeroColor;

		if (flipU)
		{
			uv->x = 1.0f - uv->x;
		}
		if (flipV)
		{
			uv->y = 1.0f - uv->y;
		}

		Vertex vertex = {};
		vertex.Position = DirectX::XMFLOAT3(position->x, position->y, position->z);
		vertex.Normal = DirectX::XMFLOAT3(normal->x, normal->y, normal->z);
		vertex.UV = DirectX::XMFLOAT2(uv->x, uv->y);
		vertex.Tangent = DirectX::XMFLOAT3(tangent->x, tangent->y, tangent->z);
		vertex.Color = DirectX::XMFLOAT4(color->r, color->g, color->b, color->a);

		dst.Vertices[i] = vertex;
    }

	dst.Indices.resize(src->mNumFaces * 3);

	for (auto i = 0u; i < src->mNumFaces; ++i)
	{
		auto face = &(src->mFaces[i]);
		assert(face->mNumIndices == 3);
		dst.Indices[i * 3 + 0] = face->mIndices[0];
		dst.Indices[i * 3 + 1] = face->mIndices[1];
		dst.Indices[i * 3 + 2] = face->mIndices[2];
	}
}

void AssimpLoader::LoadTexture(const wchar_t* filename, Mesh& dst, const aiMaterial* material)
{
	aiString path;
	if (material->Get(AI_MATKEY_TEXTURE_DIFFUSE(0), path) == AI_SUCCESS)
	{
		auto dir = GetDirectoryPath(filename);
		auto file = std::string(path.C_Str());
		auto fullPath = dir + ToWideString(file);

		dst.DiffuseMapPath = fullPath;
	}
	else
	{
		dst.DiffuseMapPath.clear();
	}
}