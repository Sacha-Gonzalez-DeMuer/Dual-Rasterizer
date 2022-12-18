#include "Texture.h"
#include "Vector2.h"
#include <SDL_image.h>

namespace dae
{
	Texture::Texture(SDL_Surface* pSurface) :
		m_pSurface{ pSurface },
		m_pSurfacePixels{ (uint32_t*)pSurface->pixels }
	{
	}

	Texture::~Texture()
	{
		if (m_pSurface)
		{
			SDL_FreeSurface(m_pSurface);
			m_pSurface = nullptr;
		}
	}

	Texture* Texture::LoadFromFile(const std::string& path)
	{
		//TODO
		//Load SDL_Surface using IMG_LOAD
		//Create & Return a new Texture Object (using SDL_Surface)
		return new Texture(IMG_Load(path.c_str()));
	}

	ColorRGB Texture::Sample(const Vector2& uv) const
	{
		//TODO
		//Sample the correct texel for the given uv
		Uint8 r{}, g{}, b{};

		//[0,1] -> [0, w/h]
		const uint32_t x{ static_cast<uint32_t>(uv.x * m_pSurface->w) };
		const uint32_t y{ static_cast<uint32_t>(uv.y * m_pSurface->h) };

		//u, v coords to idx
		const uint32_t pixel{static_cast<uint32_t>( m_pSurfacePixels[x + (y * m_pSurface->w)] ) };

		SDL_GetRGB(pixel, m_pSurface->format, &r, &g, &b);

		constexpr float toRatio{ 1.f / 255.f };
		return { r * toRatio, g * toRatio, b * toRatio };
	}
}