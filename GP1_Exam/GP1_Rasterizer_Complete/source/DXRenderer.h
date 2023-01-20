#pragma once
#include "pch.h"
#include "Renderer.h"

class DXRenderer : public Renderer
{
public:
	explicit DXRenderer(SDL_Window* pWindow, std::shared_ptr<Scene> sceneToRender);
	~DXRenderer();

	DXRenderer(const DXRenderer&) = delete;
	DXRenderer(DXRenderer&&) noexcept = delete;
	DXRenderer& operator=(const DXRenderer&) = delete;
	DXRenderer& operator=(DXRenderer&&) noexcept = delete;

	virtual void Update(Timer* pTimer) override;
	virtual void Render(const Scene& sceneToRender) override;

private:
	ID3D11Device* m_pDevice;
	ID3D11DeviceContext* m_pDeviceContext;
	IDXGISwapChain* m_pSwapChain;
	ID3D11Texture2D* m_pDepthStencilBuffer;
	ID3D11DepthStencilView* m_pDepthStencilView;
	ID3D11Texture2D* m_pRenderTargetBuffer;
	ID3D11RenderTargetView* m_pRenderTargetView;
};

