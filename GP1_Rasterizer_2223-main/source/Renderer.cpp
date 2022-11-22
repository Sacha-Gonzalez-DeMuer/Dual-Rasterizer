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

	//Render_W1_Part1();
	//Render_W1_Part2();
	Render_W1_Part3();
	//Render_W1_Part4();
	//Render_W1_Part5();

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



void Renderer::Render_W1_Part1()
{
	std::vector<Vertex> vertices_ndc
	{
		{{0.f, .5f, 1.f}},
		{{.5f, -.5f, 1.f}},
		{{-.5f, -.5f, 1.f}}
	};

	std::vector<Vertex> vertices_screenSpace{vertices_ndc};

	for (int i = 0; i < vertices_ndc.size(); ++i)
	{
		vertices_screenSpace[i].position.x = (vertices_screenSpace[i].position.x + 1) / 2 * static_cast<float>(m_Width);
		vertices_screenSpace[i].position.y = (1 - vertices_screenSpace[i].position.y) / 2 * static_cast<float>(m_Height);
	}

	//RENDER LOGIC
	for (int px{}; px < m_Width; ++px)
	{
		for (int py{}; py < m_Height; ++py)
		{
			float gradient = px / static_cast<float>(m_Width);
			gradient += py / static_cast<float>(m_Width);
			gradient /= 2.0f;

			ColorRGB finalColor{ 0, 0, 0 };


			for (size_t i = 0; i < vertices_screenSpace.size(); i += 3)
			{
				const Vector2 pixelCoord{ static_cast<float>(px), static_cast<float>(py) };

				//construct triangle
				const Vector2 v0{ vertices_screenSpace[i].position.x, vertices_screenSpace[i].position.y };
				const Vector2 v1{ vertices_screenSpace[i + 1].position.x, vertices_screenSpace[i + 1].position.y };
				const Vector2 v2{ vertices_screenSpace[i + 2].position.x, vertices_screenSpace[i + 2].position.y };
				const Vector2 vertices[3]{ v0, v1,v2 };
				const Vector2 edges[3]{
					v1 - v0,
					v2 - v1,
					v0 - v2 };

				//isPointInTri?
				Vector3 pointToSide{};
				bool isPixelInTri{ true };
				const float totalArea{ Vector2::Cross(edges[0], edges[1]) };
				for (int i = 0; i < 3; ++i)
				{
					pointToSide.x = pixelCoord.x - vertices[i].x;
					pointToSide.y = pixelCoord.y - vertices[i].y;

					const float cross{ edges[i].x * pointToSide.y - edges[i].y * pointToSide.x };
					if (cross < 0) isPixelInTri = false;
				}


				if (isPixelInTri)
				{
					finalColor = { 1,1,1 };
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
}

void Renderer::Render_W1_Part2()
{
	std::vector<Vertex> vertices_world
	{
		{{0.f, 2.f, 0.f}},
		{{1.f, .0f, .0f}},
		{{-1.f, 0.f, .0f}}
	};

	VertexTransformationFunction(vertices_world, m_vertices_screenSpace);

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
				const Vector2 pixelCoord{ static_cast<float>(px), static_cast<float>(py) };

				//construct triangle
				const Vector2 v0{ m_vertices_screenSpace[i].position.x, m_vertices_screenSpace[i].position.y };
				const Vector2 v1{ m_vertices_screenSpace[i + 1].position.x, m_vertices_screenSpace[i + 1].position.y };
				const Vector2 v2{ m_vertices_screenSpace[i + 2].position.x, m_vertices_screenSpace[i + 2].position.y };
				const Vector2 vertices[3]{ v0, v1,v2 };
				const Vector2 edges[3]{
					v1 - v0,
					v2 - v1,
					v0 - v2 };

				//isPointInTri?
				Vector3 pointToSide{};
				bool isPixelInTri{ true };
				const float totalArea{ Vector2::Cross(edges[0], edges[1]) };
				for (int i = 0; i < 3; ++i)
				{
					pointToSide.x = pixelCoord.x - vertices[i].x;
					pointToSide.y = pixelCoord.y - vertices[i].y;

					const float cross{ edges[i].x * pointToSide.y - edges[i].y * pointToSide.x };
					if (cross < 0) isPixelInTri = false;
				}


				if (isPixelInTri)
				{
					finalColor = { 1,1,1 };
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
}

void Renderer::Render_W1_Part3()
{
	std::vector<Vertex> vertices_world
	{
		//triangle 0
		{{0.f, 4.f, 2.f}, {1,0,0}},
		{{3.f, -2.0f, 2.0f}, {0,1,0}},
		{{-3.f, -2.f, 2.0f}, {0,0,1}},
	};

	VertexTransformationFunction(vertices_world, m_vertices_screenSpace);

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
				const Vector2 pixelCoord{ static_cast<float>(px), static_cast<float>(py) };

				//construct triangle
				const Vector2 v0{ m_vertices_screenSpace[i].position.x, m_vertices_screenSpace[i].position.y };
				const Vector2 v1{ m_vertices_screenSpace[i + 1].position.x, m_vertices_screenSpace[i + 1].position.y };
				const Vector2 v2{ m_vertices_screenSpace[i + 2].position.x, m_vertices_screenSpace[i + 2].position.y };
				const Vector2 vertices[3]{ v0, v1,v2 };
				const Vector2 edges[3]{
					v1 - v0,
					v2 - v1,
					v0 - v2 };

				//isPointInTri?
				Vector3 pointToSide{};
				bool isPixelInTri{ true };
				float weights[3]{};
				const float totalArea{ Vector2::Cross(edges[0], edges[1]) };
				for (int i = 0; i < 3; ++i)
				{
					pointToSide.x = pixelCoord.x - vertices[i].x;
					pointToSide.y = pixelCoord.y - vertices[i].y;

					const float cross{ edges[i].x * pointToSide.y - edges[i].y * pointToSide.x };
					weights[i] = cross / totalArea;
					if (cross < 0) isPixelInTri = false;
				}


				if (isPixelInTri)
				{
					finalColor +=
						m_vertices_screenSpace[i].color * weights[0]
						+ m_vertices_screenSpace[i + 1].color * weights[1]
						+ m_vertices_screenSpace[i + 2].color * weights[2];
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
}

void Renderer::Render_W1_Part4()
{
	std::vector<Vertex> vertices_world
	{
		//triangle 0
		{{0.f, 2.f, 0.f}, {1,0,0}},
		{{1.5f, -1.0f, 0.0f}, {1,0,0}},
		{{-1.5f, -1.f, 0.0f}, {1,0,0}},


		//triangle 1
		{{0.f, 4.f, 2.f}, {1,0,0}},
		{{3.f, -2.f, 2.f}, {0,1,0}},
		{{-3.f, -2.f, 2.f}, {0,0,1}}
	};

	VertexTransformationFunction(vertices_world, m_vertices_screenSpace);

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
				const Vector2 pixelCoord{ static_cast<float>(px), static_cast<float>(py) };

				//construct triangle
				const Vector2 v0{ m_vertices_screenSpace[i].position.x, m_vertices_screenSpace[i].position.y };
				const Vector2 v1{ m_vertices_screenSpace[i + 1].position.x, m_vertices_screenSpace[i + 1].position.y };
				const Vector2 v2{ m_vertices_screenSpace[i + 2].position.x, m_vertices_screenSpace[i + 2].position.y };
				const Vector2 vertices[3]{ v0, v1,v2 };
				const Vector2 edges[3]{
					v1 - v0,
					v2 - v1,
					v0 - v2 };

				//isPointInTri?
				Vector3 pointToSide{};
				bool isPixelInTri{ true };
				float weights[3]{};
				const float totalArea{ Vector2::Cross(edges[0], edges[1]) };
				for (int i = 0; i < 3; ++i)
				{
					pointToSide.x = pixelCoord.x - vertices[i].x;
					pointToSide.y = pixelCoord.y - vertices[i].y;

					const float cross{ edges[i].x * pointToSide.y - edges[i].y * pointToSide.x };
					weights[i] = cross / totalArea;
					if (cross < 0) isPixelInTri = false;
				}


				if (isPixelInTri)
				{
					finalColor +=
						m_vertices_screenSpace[i].color * weights[0]
						+ m_vertices_screenSpace[i + 1].color * weights[1]
						+ m_vertices_screenSpace[i + 2].color * weights[2];
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
}

void Renderer::Render_W1_Part5()
{
	
}





