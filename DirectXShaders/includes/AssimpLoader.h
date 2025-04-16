#pragma once
#define NOMINMAX
#include <d3d12.h>
#include <DirectXMath.h>
#include <string>
#include <vector>

struct Mesh;
struct Vertex;

struct aiMesh;
struct aiMaterial;

struct ImportSettings
{
	const wchar_t* filename = nullptr;
	std::vector<Mesh>& meshes;
	bool flipU = false;
	bool flipV = false;
};

class AssimpLoader
{
public:
	bool Load(ImportSettings settings);

private:
	void LoadMesh(Mesh& dst, const aiMesh* src, bool flipU, bool flipV);
	void LoadTexture(const wchar_t* filename, Mesh& dst, const aiMaterial* material);
};