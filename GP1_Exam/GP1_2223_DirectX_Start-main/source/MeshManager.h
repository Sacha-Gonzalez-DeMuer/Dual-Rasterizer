#pragma once
#include <unordered_map>
#include "Mesh.h"

class MeshManager
{
public:
	static MeshManager* Get();
	MeshManager();

	MeshManager(const MeshManager& other) = delete;
	MeshManager(MeshManager&& other) = delete;
	MeshManager& operator=(const MeshManager& other) = delete;
	MeshManager& operator=(MeshManager&& other) = delete;

	std::shared_ptr<Mesh> GetMesh(const std::string& filePath);


private:
	std::unordered_map<std::string, std::shared_ptr<Mesh>> m_MeshDictionary;
};