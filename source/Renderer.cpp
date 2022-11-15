//External includes
#include "SDL.h"
#include "SDL_surface.h"

//Project includes
#include "Renderer.h"
#include "Math.h"
#include "Matrix.h"
#include "Texture.h"
#include "Utils.h"

using namespace dae;

Renderer::Renderer(SDL_Window* pWindow) :
	m_pWindow(pWindow)
{
	//Initialize
	SDL_GetWindowSize(pWindow, &m_Width, &m_Height);

	//Create Buffers
	m_pFrontBuffer = SDL_GetWindowSurface(pWindow);
	m_pBackBuffer = SDL_CreateRGBSurface(0, m_Width, m_Height, 32, 0, 0, 0, 0);
	m_pBackBufferPixels = (uint32_t*)m_pBackBuffer->pixels;

	//m_pDepthBufferPixels = new float[m_Width * m_Height];

	//Initialize Camera
	m_Camera.Initialize(60.f, { .0f,.0f,-10.f });
}

Renderer::~Renderer()
{
	//delete[] m_pDepthBufferPixels;
}

void Renderer::Update(Timer* pTimer)
{
	m_Camera.Update(pTimer);
}

void Renderer::Render()
{
	//@START
	//Lock BackBuffer
	SDL_LockSurface(m_pBackBuffer);
	

	const float fWidth{ static_cast<float>(m_Width) };
	const float fHeight{ static_cast<float>(m_Height) };

	VertexTransformationFunction(m_vertices_world, m_vertices_screenSpace);

	//RENDER LOGIC
	for (int px{}; px < m_Width; ++px)
	{
		for (int py{}; py < m_Height; ++py)
		{
			float gradient = px / fWidth;
			gradient += py / fWidth;
			gradient /= 2.0f;
			ColorRGB finalColor{ gradient, gradient, gradient };


			const Vector2 v0{ m_vertices_world[0].position.x, m_vertices_screenSpace[0].position.y };
			const Vector2 v1{ m_vertices_world[1].position.x, m_vertices_screenSpace[1].position.y };
			const Vector2 v2{ m_vertices_world[2].position.x, m_vertices_screenSpace[2].position.y };
		   
			const Vector2 pixelCoord{ static_cast<float>(px), static_cast<float>(py) };
			const Vector2 vertices[3]{ {v0.x, v0.y}, {v1.x, v1.y},{v2.x, v2.y} };

			const Vector2 edges[3] {
				v1 - v0, 
				v2 - v1, 
				v0 - v2 };


			Vector3 pointToSide{};
			bool isPixelInTri{ true };
			for (int i = 0; i < 3; ++i)
			{
				pointToSide.x =  pixelCoord.x - vertices[i].x;
				pointToSide.y = pixelCoord.y - vertices[i].y;

				if (edges[i].x * pointToSide.y - edges[i].y * pointToSide.x < 0) isPixelInTri = false;
			}

			if (isPixelInTri) finalColor = { 1,1,1 };
			else finalColor = { 0,0,0 };


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

void Renderer::VertexTransformationFunction(const std::vector<Vertex>& vertices_in, std::vector<Vertex>& vertices_out) const
{
	//Todo > W1 Projection Stage
	for (int i = 0; i < vertices_in.size(); ++i)
	{
		Vertex screenSpaceVertex{ vertices_in[i] };
		//transform vertices with view matrix
		m_Camera.invViewMatrix.TransformPoint(screenSpaceVertex.position);

		//apply perspective divide
		screenSpaceVertex.position.y = screenSpaceVertex.position.y / screenSpaceVertex.position.z;


		//convert to screen space
		Vertex v{ screenSpaceVertex };

		v.position.x = (screenSpaceVertex.position.x + 1) / 2 * static_cast<float>(m_Width);
		v.position.y = (1 - screenSpaceVertex.position.y) / 2 * static_cast<float>(m_Height);

		vertices_out.emplace_back(v);
	}
}

bool Renderer::SaveBufferToImage() const
{
	return SDL_SaveBMP(m_pBackBuffer, "Rasterizer_ColorBuffer.bmp");
}
