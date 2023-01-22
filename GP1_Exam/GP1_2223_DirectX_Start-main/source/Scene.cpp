#include "pch.h"
#include "Scene.h"
#include "Mesh.h"
#include "Material.h"
#include "SRenderer.h"
#include "DXRenderer.h"

Scene::Scene()
	: m_Camera{ { 0.f, 0.f, -10.f }, 45.f, 1.7777f }
{
}

Scene::~Scene()
{
}

void Scene::Update(const dae::Timer* pTimer)
{
	m_Camera.Update(pTimer);
	for (const auto& pMesh : m_pMeshes)
	{
		pMesh->Update(pTimer);
	}
}

void Scene::DXRender(ID3D11Device* pDevice, ID3D11DeviceContext* pDeviceContext)
{
	for (const auto& pMesh : m_pMeshes)
	{
		pMesh->DXRender(pDevice, pDeviceContext, m_Camera);
	}
}

std::shared_ptr<Mesh> Scene::AddMesh(std::shared_ptr<Mesh> pMesh)
{
	m_pMeshes.push_back(pMesh);
	return pMesh;
}
