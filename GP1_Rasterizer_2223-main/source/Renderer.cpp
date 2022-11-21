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
	m_Camera.Initialize(60.f, { .0f,.0f, -10.f });
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

	VertexTransformationFunction(m_vertices_world, m_vertices_screenSpace);

	//RENDER LOGIC
	for (int px{}; px < m_Width; ++px)
	{
		for (int py{}; py < m_Height; ++py)
		{
			float gradient = px / static_cast<float>(m_Width);
			gradient += py / static_cast<float>(m_Width);
			gradient /= 2.0f;

			ColorRGB finalColor{ 0, 0, 0 };


			for (size_t i = 0; i < m_vertices_screenSpace.size(); i += 3)
			{
				const Vector2 v0{ m_vertices_screenSpace[i].position.x, m_vertices_screenSpace[i].position.y };
				const Vector2 v1{ m_vertices_screenSpace[i+1].position.x, m_vertices_screenSpace[i+1].position.y };
				const Vector2 v2{ m_vertices_screenSpace[i+2].position.x, m_vertices_screenSpace[i+2].position.y };

				const Vector2 pixelCoord{ static_cast<float>(px), static_cast<float>(py) };
				const Vector2 vertices[3]{ {v0.x, v0.y}, {v1.x, v1.y},{v2.x, v2.y} };

				const Vector2 edges[3]{
					v1 - v0,
					v2 - v1,
					v0 - v2 };

				Vector3 pointToSide{};
				bool isPixelInTri{ true };
				for (int i = 0; i < 3; ++i)
				{
					pointToSide.x = pixelCoord.x - vertices[i].x;
					pointToSide.y = pixelCoord.y - vertices[i].y;

					if (edges[i].x * pointToSide.y - edges[i].y * pointToSide.x < 0) isPixelInTri = false;
				}

				if (isPixelInTri)
				{
					float W0 = -(Vector2::Cross((pixelCoord - v1), (v2 - v1)) / 2);
					float W1 = -(Vector2::Cross((pixelCoord - v2), (v0 - v2)) / 2);
					float W2 = -(Vector2::Cross((pixelCoord - v0), (v1 - v0)) / 2);

					finalColor += m_vertices_screenSpace[i].color	* W0;
					finalColor += m_vertices_screenSpace[i+1].color * W1;
					finalColor += m_vertices_screenSpace[i+2].color	* W2;
				}
			}

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
	float aspectRatio{ static_cast<float>(m_Width) / static_cast<float>(m_Height) };
	vertices_out = vertices_in;
	//Todo > W1 Projection Stage
	for (int i = 0; i < vertices_in.size(); ++i)
	{
		//transform vertices to viewSpace
		vertices_out[i].position = m_Camera.viewMatrix.TransformPoint(vertices_in[i].position);

		//apply perspective divide
		vertices_out[i].position.x = vertices_out[i].position.x / vertices_out[i].position.z;
		vertices_out[i].position.y = vertices_out[i].position.y / vertices_out[i].position.z;

		//project
		vertices_out[i].position.x = vertices_out[i].position.x / (aspectRatio * m_Camera.fov);
		vertices_out[i].position.y = vertices_out[i].position.y / m_Camera.fov;

		//to screen space
		vertices_out[i].position.x = (vertices_out[i].position.x + 1) / 2 * static_cast<float>(m_Width);
		vertices_out[i].position.y = (1 - vertices_out[i].position.y) / 2 * static_cast<float>(m_Height);
	}
}

bool Renderer::SaveBufferToImage() const
{	
	return SDL_SaveBMP(m_pBackBuffer, "Rasterizer_ColorBuffer.bmp");
}
