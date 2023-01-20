#include "pch.h"
#include "SRenderer.h"


SRenderer::SRenderer(SDL_Window* pWindow)
	: Renderer(pWindow)
{
	m_pDepthBufferPixels = new float[m_Width * m_Height];

	//Create Buffers
	m_pFrontBuffer = SDL_GetWindowSurface(pWindow);
	m_pBackBuffer = SDL_CreateRGBSurface(0, m_Width, m_Height, 32, 0, 0, 0, 0);
	m_pBackBufferPixels = (uint32_t*)m_pBackBuffer->pixels;
}

SRenderer::~SRenderer()
{
	delete m_pFrontBuffer;
	m_pFrontBuffer = nullptr;

	delete m_pBackBuffer;
	m_pBackBuffer = nullptr;

	delete m_pBackBufferPixels;
	m_pBackBufferPixels = nullptr;
}

void SRenderer::Render()
{
	//@START
	//Lock BackBuffer
	SDL_LockSurface(m_pBackBuffer);

	//RENDER LOGIC
	for (int px{}; px < m_Width; ++px)
	{
		for (int py{}; py < m_Height; ++py)
		{
			float gradient = px / static_cast<float>(m_Width);
			gradient += py / static_cast<float>(m_Width);
			gradient /= 2.0f;

			dae::ColorRGB finalColor{ gradient, gradient, gradient };

			//Update Color in Buffer
			finalColor.MaxToOne();

			m_pBackBufferPixels[px + (py * m_Width)] = SDL_MapRGB(m_pBackBuffer->format,
				static_cast<uint8_t>(finalColor.r * 255),
				static_cast<uint8_t>(finalColor.g * 255),
				static_cast<uint8_t>(finalColor.b * 255));
		}
	}

	//@END
	//Update SDL Surface
	SDL_UnlockSurface(m_pBackBuffer);
	SDL_BlitSurface(m_pBackBuffer, 0, m_pFrontBuffer, 0);
	SDL_UpdateWindowSurface(m_pWindow);
}

void SRenderer::Update(dae::Timer* pTimer)
{
}


void SRenderer::VertexTransformationFunction(const std::vector<dae::Vertex>& vertices_in, std::vector<dae::Vertex>& vertices_out) const
{
}
