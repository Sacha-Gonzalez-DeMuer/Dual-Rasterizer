#include "pch.h"
#include "DXRenderer.h"
#include "ColorRGB.h"
#include "Scene.h"

DXRenderer::DXRenderer(SDL_Window* pWindow)
	: Renderer(pWindow)
{
	//Initialize DirectX pipeline
	const HRESULT result = InitializeDirectX();
	if (result == S_OK)
	{
		m_IsInitialized = true;
		std::cout << "DirectX is initialized and ready!\n";
	}
	else
	{
		std::cout << "DirectX initialization failed!\n";
	}
}

DXRenderer::~DXRenderer()
{
	std::cout << "Hardware Destructor\n";

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

void DXRenderer::Update(dae::Timer* pTimer)
{
	m_pScene->Update(pTimer);
}

void DXRenderer::Render()
{
	if (!m_IsInitialized)
		return;

	//1. Clear RTV & DSV
	dae::ColorRGB clearColor{ m_ClearColor ? BACKGROUND_HARDWARE : BACKGROUND_CLEAR };

	m_pDeviceContext->ClearRenderTargetView(m_pRenderTargetView, &clearColor.r);
	m_pDeviceContext->ClearDepthStencilView(m_pDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.f, 0);

	//2. Set pipeline + invoke drawcalls (= RENDER)
	//...
	m_pScene->DXRender(m_pDevice, m_pDeviceContext);

	//3. Present backbuffer (SWAP)
	m_pSwapChain->Present(0, 0);
}

HRESULT DXRenderer::InitializeDirectX()
{
	//1. Create Device & DeviceContext
		//=====
	D3D_FEATURE_LEVEL featureLevel{ D3D_FEATURE_LEVEL_11_1 };
	uint32_t createDeviceFlags{ 0 }; //runtime layers to enable; values can be bitwise OR'd together.

#if defined(DEBUG)||defined(_DEBUG)
	createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	HRESULT result{ D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, 0, createDeviceFlags, &featureLevel, 1, D3D11_SDK_VERSION, &m_pDevice, nullptr, &m_pDeviceContext) };
	if (FAILED(result))
		return result;

	//Create DXGI Factory (needed to set up swapchain)
	IDXGIFactory1* pDxgiFactory{};
	result = CreateDXGIFactory1(__uuidof(IDXGIFactory1), reinterpret_cast<void**>(&pDxgiFactory));
	if (FAILED(result))
		return result;

	//2. Create Swapchain
	//=====
	DXGI_SWAP_CHAIN_DESC swapChainDesc{}; //https://learn.microsoft.com/en-us/windows/win32/api/dxgi/ns-dxgi-dxgi_swap_chain_desc

	//BufferDesc; Describes a display mode.
	swapChainDesc.BufferDesc.Width = m_Width;
	swapChainDesc.BufferDesc.Height = m_Height;
	swapChainDesc.BufferDesc.RefreshRate.Numerator = 1;
	swapChainDesc.BufferDesc.RefreshRate.Denominator = 60;
	swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; //structure describing the display format
	swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED; //flag indicating method the raster uses to create an image on a surface.
	swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED; //flag indicating how an image is stretched to fit a given monitor's resolution.

	//SampleDesc; Describes multi-sampling parameters for a resource.
	swapChainDesc.SampleDesc.Count = 1; //number of multisamples per pixel.
	swapChainDesc.SampleDesc.Quality = 0;  //quality of multisampling

	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT; //use the surface/resource as an output render target
	swapChainDesc.BufferCount = 1; //number of buffers in swap chain (only front buffer)
	swapChainDesc.Windowed = true;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD; //contents of the backbuffer are discarded after they are presented to the front buffer
	swapChainDesc.Flags = 0;


	//Get the handle (HWND) from the SDL Backbuffer
	SDL_SysWMinfo sysWMInfo{};
	SDL_VERSION(&sysWMInfo.version);
	SDL_GetWindowWMInfo(m_pWindow, &sysWMInfo);
	swapChainDesc.OutputWindow = sysWMInfo.info.win.window;

	//Create SwapChain
	result = pDxgiFactory->CreateSwapChain(m_pDevice, &swapChainDesc, &m_pSwapChain);
	if (FAILED(result))
		return result;

	//3. Create DepthStencil (DS) & DepthStencilView (DVS)
	//=====
	//Resource
	D3D11_TEXTURE2D_DESC depthStencilDesc{};
	depthStencilDesc.Width = m_Width;
	depthStencilDesc.Height = m_Height;
	depthStencilDesc.MipLevels = 1; //nr of mipmap levels to be used for texture. Mipmap = pre-calculated optimized set of texture maps, allows texture to be rendered at different sizes w/o losing detail or introducing distortion
	depthStencilDesc.ArraySize = 1; //nr of textures in texture array
	depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencilDesc.SampleDesc.Count = 1;
	depthStencilDesc.SampleDesc.Quality = 0;
	depthStencilDesc.Usage = D3D11_USAGE_DEFAULT; //define how the texture is to be read from and written to
	depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthStencilDesc.CPUAccessFlags = 0; //cpu access not required
	depthStencilDesc.MiscFlags = 0;

	//View
	D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc{};
	depthStencilViewDesc.Format = depthStencilDesc.Format;
	depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	depthStencilViewDesc.Texture2D.MipSlice = 0;

	result = m_pDevice->CreateTexture2D(&depthStencilDesc, nullptr, &m_pDepthStencilBuffer);
	if (FAILED(result))
		return result;

	result = m_pDevice->CreateDepthStencilView(m_pDepthStencilBuffer, &depthStencilViewDesc, &m_pDepthStencilView);
	if (FAILED(result))
		return result;


	//4. Create RenderTarget (RT) & RenderTargetView (RTV)
	//=====

	//Resource
	result = m_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&m_pRenderTargetBuffer));
	if (FAILED(result))
		return result;

	//View
	result = m_pDevice->CreateRenderTargetView(m_pRenderTargetBuffer, nullptr, &m_pRenderTargetView);
	if (FAILED(result))
		return result;

	//5. Bind RTV & DSV to Output Merger Stage
	//=====
	m_pDeviceContext->OMSetRenderTargets(1, &m_pRenderTargetView, m_pDepthStencilView);


	//6. Set Viewport
	//=====
	D3D11_VIEWPORT viewport{};
	viewport.Width = static_cast<float>(m_Width);
	viewport.Height = static_cast<float>(m_Height);
	viewport.TopLeftX = 0.f;
	viewport.TopLeftY = 0.f;
	viewport.MinDepth = 0.f;
	viewport.MaxDepth = 1.f;
	m_pDeviceContext->RSSetViewports(1, &viewport);

	pDxgiFactory->Release();
	pDxgiFactory = nullptr;

	return result;
}
