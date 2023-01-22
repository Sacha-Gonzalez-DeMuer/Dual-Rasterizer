#include "pch.h"
#include "DXRenderer.h"
#include "SRenderer.h"
#include "VehicleScene.h"
#include "CombinedRenderer.h"
#include "FilePaths.h"

CombinedRenderer::CombinedRenderer(SDL_Window* pWindow)
	: m_UseGPU{ false }
	, m_RotateMesh{ true }
	, m_CullMode{ CullMode::Backface }
	, Renderer(pWindow)
	, SRenderer(pWindow)
	, DXRenderer(pWindow)
{
	auto scene{ std::make_shared<VehicleScene>() };
	// Initialize meshes and materials in scene
	scene->Initialize(m_pDevice);
	scene->GetCamera().SetAspectRatio(static_cast<float>(m_Width) / static_cast<float>(m_Height));

	//TODO: clean this up
	SetSceneToRender(scene);
	SRenderer::SetSceneToRender(scene);
	DXRenderer::SetSceneToRender(scene);
}


void CombinedRenderer::Update(dae::Timer* pTimer)
{
	m_pScene->Update(pTimer);
}

void CombinedRenderer::Render()
{
	if (m_UseGPU)
		DXRenderer::Render();
	else
		SRenderer::Render();
}

void CombinedRenderer::ToggleGPU()
{
	m_UseGPU = !m_UseGPU;
}

void CombinedRenderer::ToggleRotation()
{
	for (auto& mesh : m_pScene->GetMeshes())
	{
		mesh->ToggleRotation();
	}
}

void CombinedRenderer::CycleCullMode()
{

	int current = static_cast<int>(m_CullMode);
	++current;
	current = current % static_cast<int>(CullMode::Size);
	m_CullMode = static_cast<CullMode>(current);

	std::cout << "Cull mode: ";
	switch (m_CullMode)
	{
	case CullMode::Backface:
		std::cout << "BACKFACE\n";
		break;
	case CullMode::Frontface:
		std::cout << "FRONTFACE\n";
		break;
	case CullMode::None:
		std::cout << "NONE\n";
		break;
	}

	for (auto& mesh : m_pScene->GetMeshes())
	{
		mesh->SetCullMode(m_CullMode, m_pDevice, m_pDeviceContext);
	}
}
\