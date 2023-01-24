#include "pch.h"
#include "DXRenderer.h"
#include "SRenderer.h"
#include "VehicleScene.h"
#include "CombinedRenderer.h"
#include "FilePaths.h"
#include "Utils.h"

CombinedRenderer::CombinedRenderer(SDL_Window* pWindow)
	: m_UseGPU{ false }
	, m_RotateMesh{ true }
	, m_CullMode{ CullMode::Backface }
	, m_SampleState{ SamplerState::Linear }
	, m_ShadingMode{ ShadingMode::Combined }
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

	std::cout << "Using renderer: ";
	m_UseGPU ? std::cout << "HARDWARE\n" : std::cout << "SOFTWARE\n";
}

void CombinedRenderer::ToggleRotation()
{
	m_RotateMesh = !m_RotateMesh;

	for (auto& mesh : m_pScene->GetMeshes())
	{
		mesh->SetRotation(m_RotateMesh);
	}

	std::cout << "Mesh Rotation: ";
	m_RotateMesh ? std::cout << "ON\n" : std::cout << "OFF\n";
}

void CombinedRenderer::ToggleNormalMap()
{
	m_UseNormalMaps = !m_UseNormalMaps;
	for (auto& mat : MaterialManager::Get()->GetMaterials())
	{
		mat.second->UseNormalMap(m_UseNormalMaps);
	}

	std::cout << "Normal maps: ";
	m_UseNormalMaps ? std::cout << "ON\n" : std::cout << "OFF\n";
}

void CombinedRenderer::ToggleDepthBuffer()
{
	m_VisualizeDepthBuffer = !m_VisualizeDepthBuffer;
	SRenderer::SetDepthBufferVisualization(m_VisualizeDepthBuffer);

	std::cout << "Depth buffer visualization: ";
	m_VisualizeDepthBuffer ? std::cout << "ON\n" : std::cout << "OFF\n";
}

void CombinedRenderer::ToggleBoundingBox()
{
	m_VisualizeBoundingBox = !m_VisualizeBoundingBox;
	SRenderer::SetBoundingBoxVisualization(m_VisualizeBoundingBox);


	std::cout << "Bounding box visualization: ";
	m_VisualizeBoundingBox ? std::cout << "ON\n" : std::cout << "OFF\n";
}


void CombinedRenderer::CycleCullMode()
{
	dae::Utils::CycleEnum<CullMode>(m_CullMode);

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


	SRenderer::SetCullMode(m_CullMode);
}
void CombinedRenderer::CycleSampleState()
{
	dae::Utils::CycleEnum<SamplerState>(m_SampleState);

	std::cout << "Sample state: ";
	switch (m_SampleState)
	{
	case SamplerState::Point:
		std::cout << "POINT\n";
		break;
	case SamplerState::Linear:
		std::cout << "LINEAR\n";
		break;
	case SamplerState::Anisotropic:
		std::cout << "ANISOTROPIC\n";
		break;
	}

	for (auto& mesh : m_pScene->GetMeshes())
	{
		mesh->SetSampleState(m_SampleState, m_pDevice, m_pDeviceContext);
	}
}

void CombinedRenderer::CycleShadingMode()
{
	dae::Utils::CycleEnum<ShadingMode>(m_ShadingMode);

	std::cout << "Shading mode: ";
	switch (m_ShadingMode)
	{
	case ShadingMode::Combined:
		std::cout << "COMBINED\n";
		break;
	case ShadingMode::ObservedArea:
		std::cout << "OBSERVED AREA\n";
		break;
	case ShadingMode::Diffuse:
		std::cout << "DIFFUSE\n";
		break;

	case ShadingMode::Specular:
		std::cout << "SPECULAR\n";
		break;
	}

	for (auto& mat : MaterialManager::Get()->GetMaterials())
	{
		mat.second->SetShadingMode(m_ShadingMode);
	}
}

void CombinedRenderer::BoostMovementSpeed()
{
	m_pScene->GetCamera().BoostMovementSpeed();
}
