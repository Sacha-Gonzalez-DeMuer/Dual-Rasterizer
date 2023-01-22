#include "pch.h"
#include "Texture.h"
#include "Vector2.h"
#include "ColorRGB.h"

Texture::Texture(ID3D11Device* pDevice, SDL_Surface* pSurface)
	: m_pSurface{pSurface}
	, m_pSurfacePixels{ (uint32_t*)pSurface->pixels }
{
	Initialize(pDevice, pSurface);
}

Texture::~Texture()
{
	if (m_pResource)
	{
		m_pResource->Release();
		m_pResource = nullptr;
	}

	if (m_pShaderResourceView)
	{
		m_pShaderResourceView->Release();
		m_pShaderResourceView = nullptr;
	}

	if (m_pSurface)
	{
		SDL_FreeSurface(m_pSurface);
		m_pSurface = nullptr;
	}
}

std::shared_ptr<Texture> Texture::LoadFromFile(ID3D11Device* pDevice, const std::string& path)
{
	return std::make_shared<Texture>(pDevice, IMG_Load(path.c_str()));
}

dae::ColorRGB Texture::Sample(const dae::Vector2& uv) const
{
	//Sample the correct texel for the given uv
	Uint8 r{}, g{}, b{};

	//[0,1] -> [0, w/h]
	const uint32_t x{ static_cast<uint32_t>(uv.x * m_pSurface->w) };
	const uint32_t y{ static_cast<uint32_t>(uv.y * m_pSurface->h) };

	//u, v coords to idx
	const uint32_t pixel{ static_cast<uint32_t>(m_pSurfacePixels[x + (y * m_pSurface->w)]) };

	SDL_GetRGB(pixel, m_pSurface->format, &r, &g, &b);

	constexpr float toRatio{ 1.f / 255.f };
	return { r * toRatio, g * toRatio, b * toRatio };
}

void Texture::Initialize(ID3D11Device* pDevice, SDL_Surface* pSurface)
{
	// Create 2d texture resource
	DXGI_FORMAT format = DXGI_FORMAT_R8G8B8A8_UNORM;
	D3D11_TEXTURE2D_DESC desc{};
	desc.Width = pSurface->w;
	desc.Height = pSurface->h;
	desc.MipLevels = 1;
	desc.ArraySize = 1;
	desc.Format = format;
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	desc.CPUAccessFlags = 0;
	desc.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA initData;
	initData.pSysMem = pSurface->pixels;
	initData.SysMemPitch = static_cast<UINT>(pSurface->pitch);
	initData.SysMemSlicePitch = static_cast<UINT>(pSurface->h * pSurface->pitch);

	HRESULT result = pDevice->CreateTexture2D(&desc, &initData, &m_pResource);

	if (FAILED(result))
	{
		std::cout << "Failed to create m_pResource. Texture.cpp l51\n";
		return;
	}

	//create view for resource
	D3D11_SHADER_RESOURCE_VIEW_DESC SRVDesc{};
	SRVDesc.Format = format;
	SRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	SRVDesc.Texture2D.MipLevels = 1;

	result = pDevice->CreateShaderResourceView(m_pResource, &SRVDesc, &m_pShaderResourceView);


	if (FAILED(result))
	{
		std::cout << "Failed to create resource view. Texture.cpp l66\n";
		return;
	}
}