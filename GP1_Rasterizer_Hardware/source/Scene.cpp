#include "pch.h"
#include "Scene.h"
#include "Mesh.h"
#include "Material.h"

Scene::Scene()
	: m_Camera{ { 0.f, 0.f, -10.f }, 45.f, 1.7777f }
{
}

Scene::~Scene()
{
	for (auto& mesh : m_pMeshes)
	{
		delete mesh;
		mesh = nullptr;
	}
	m_pMeshes.clear();

	for (auto& mesh : m_pMaterials)
	{
		delete mesh;
		mesh = nullptr;
	}
	m_pMaterials.clear();
}

void Scene::Update(const dae::Timer* pTimer)
{
	m_Camera.Update(pTimer);
}

void Scene::AddMesh(ID3D11Device* pDevice, ID3D11DeviceContext* pDeviceContext, const std::string& filePath, const std::shared_ptr<Material>& mat)
{
	Mesh* newMesh{new Mesh(pDevice, pDeviceContext, filePath, mat)};
	m_pMeshes.push_back(newMesh);
}

void Scene::AddMaterial(Material* pMat)
{
	m_pMaterials.push_back(pMat);
}

void Scene::Render(ID3D11Device* pDevice, ID3D11DeviceContext* pDeviceContext)
{
	for (const auto& pMesh : m_pMeshes)
	{
		pMesh->Render(pDevice, pDeviceContext, m_Camera);
	}
}
