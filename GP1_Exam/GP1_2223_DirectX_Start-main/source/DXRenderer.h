#pragma once
#include "pch.h"
#include "Renderer.h"

class DXRenderer : virtual public Renderer
{
public:
	explicit DXRenderer(SDL_Window* pWindow);
	virtual ~DXRenderer() override;

	DXRenderer(const DXRenderer&) = delete;
	DXRenderer(DXRenderer&&) noexcept = delete;
	DXRenderer& operator=(const DXRenderer&) = delete;
	DXRenderer& operator=(DXRenderer&&) noexcept = delete;

	virtual void Update(dae::Timer* pTimer) override;
	virtual void Render() override;

protected:
	bool m_IsInitialized{ false };

	ID3D11Device* m_pDevice;
	ID3D11DeviceContext* m_pDeviceContext;
	IDXGISwapChain* m_pSwapChain;
	ID3D11Texture2D* m_pDepthStencilBuffer;
	ID3D11DepthStencilView* m_pDepthStencilView;
	ID3D11Texture2D* m_pRenderTargetBuffer;
	ID3D11RenderTargetView* m_pRenderTargetView;

	HRESULT InitializeDirectX();
	
};

