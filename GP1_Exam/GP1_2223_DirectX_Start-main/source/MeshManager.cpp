#include "pch.h"
#include "MeshManager.h"

MeshManager* MeshManager::Get()
{
    static MeshManager* m = new MeshManager();

    return m;
}

MeshManager::MeshManager()
    :m_MeshDictionary{}
{
}

std::shared_ptr<Mesh> MeshManager::GetMesh(const std::string& filePath)
{
   if (!m_MeshDictionary.contains(filePath))
   {
      m_MeshDictionary.insert(std::make_pair(filePath, std::make_shared<Mesh>(filePath)));
   }

   return m_MeshDictionary[filePath];
}