#include "pch.h"
#include "DXRenderer.h"
#include "SRenderer.h"
#include "VehicleScene.h"
#include "CombinedRenderer.h"

CombinedRenderer::CombinedRenderer(SDL_Window* pWindow)
	: m_UseGPU{true}
	, Renderer(pWindow)
	, SRenderer(pWindow)
	, DXRenderer(pWindow)
{
	auto scene{ std::make_shared<VehicleScene>() };
	scene->Initialize(m_pDevice);


	scene->GetCamera().SetAspectRatio(static_cast<float>(m_Width) / static_cast<float>(m_Height));
	SetSceneToRender(scene);
	SRenderer::SetSceneToRender(scene);
	DXRenderer::SetSceneToRender(scene);
}

CombinedRenderer::~CombinedRenderer()
{
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
