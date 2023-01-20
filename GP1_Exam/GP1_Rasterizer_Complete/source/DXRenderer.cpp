#include "DXRenderer.h"

DXRenderer::DXRenderer(SDL_Window* pWindow, std::shared_ptr<Scene> sceneToRender)
	: Renderer(pWindow, sceneToRender)
{

}

DXRenderer::~DXRenderer()
{
	m_pRenderTargetView->Release();
	m_pRenderTargetBuffer->Release();
	m_pDepthStencilView->Release();
	m_pDepthStencilBuffer->Release();
	m_pSwapChain->Release();
	m_pDeviceContext->Release();
	m_pDevice->Release();

	m_pRenderTargetView = nullptr;
	m_pRenderTargetBuffer = nullptr;
	m_pDepthStencilBuffer = nullptr;
	m_pDepthStencilView = nullptr;
	m_pSwapChain = nullptr;
	m_pDeviceContext = nullptr;
	m_pDevice = nullptr;
}

void DXRenderer::Update(Timer* pTimer)
{

}

void DXRenderer::Render(const Scene& sceneToRender)
{

}
