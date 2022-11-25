//External includes
#include "SDL.h"
#include "SDL_surface.h"
#include <algorithm>
#include <iostream>

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

	m_pDepthBufferPixels = new float[m_Width * m_Height];


	//Initialize Camera
	m_Camera.Initialize(60.f, { .0f,.0f, -10.f });
	m_pTexture = Texture::LoadFromFile("Resources/uv_grid_2.png");
}

Renderer::~Renderer()
{
	delete[] m_pDepthBufferPixels;
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
	//Render_W1_Part3();
	//Render_W1_Part4();
	//Render_W1_Part5();

	//Render_W2_Part1(); //triangle list&strip
	Render_W2_Part2();	//uv
	//Render_W2_Part2_2(); //uv, reworking mesh parse

	//@END
	//Update SDL Surface
	SDL_UnlockSurface(m_pBackBuffer);
	SDL_BlitSurface(m_pBackBuffer, 0, m_pFrontBuffer, 0);
	SDL_UpdateWindowSurface(m_pWindow);
}






bool Renderer::SaveBufferToImage() const
{	
	return SDL_SaveBMP(m_pBackBuffer, "Rasterizer_ColorBuffer.bmp");
}


#pragma region Week1
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

	//depth buffer
	std::fill_n(m_pDepthBufferPixels, m_Width * m_Height, FLT_MAX);
	SDL_FillRect(m_pBackBuffer, NULL, SDL_MapRGB(m_pBackBuffer->format, 100, 100, 100));


	VertexTransformationFunction(vertices_world, m_vertices_screenSpace);

	//RENDER LOGIC
	for (int px{}; px < m_Width; ++px)
	{
		for (int py{}; py < m_Height; ++py)
		{
			const int pixelIdx{ px + py * m_Width };

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
					//interpolate depth values
					const float interpolatedDepth{
						m_vertices_screenSpace[i].position.z * weights[0]
						+ m_vertices_screenSpace[i + 1].position.z * weights[1]
						+ m_vertices_screenSpace[i + 2].position.z * weights[2] };


					if (interpolatedDepth < m_pDepthBufferPixels[pixelIdx])
					{
						m_pDepthBufferPixels[pixelIdx] = interpolatedDepth;

						//color pixel according to vertex colors
						finalColor +=
							m_vertices_screenSpace[i].color * weights[0]
							+ m_vertices_screenSpace[i + 1].color * weights[1]
							+ m_vertices_screenSpace[i + 2].color * weights[2];
					}
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

	//depth buffer
	std::fill_n(m_pDepthBufferPixels, m_Width * m_Height, FLT_MAX);
	SDL_FillRect(m_pBackBuffer, NULL, SDL_MapRGB(m_pBackBuffer->format, 100, 100, 100));

	VertexTransformationFunction(vertices_world, m_vertices_screenSpace);
	//for every tri
	for (size_t i = 0; i < m_vertices_screenSpace.size(); i += 3)
	{
		//construct triangle
		const Vector2 v0{ m_vertices_screenSpace[i].position.x, m_vertices_screenSpace[i].position.y };
		const Vector2 v1{ m_vertices_screenSpace[i + 1].position.x, m_vertices_screenSpace[i + 1].position.y };
		const Vector2 v2{ m_vertices_screenSpace[i + 2].position.x, m_vertices_screenSpace[i + 2].position.y };
		const Vector2 vertices[3]{ v0, v1,v2 };
		const Vector2 edges[3]{
			v1 - v0,
			v2 - v1,
			v0 - v2 };

		const BoundingBox boundingBox{ GenerateBoundingBox(vertices) };

		//every pixel
		for (int px{boundingBox.minX}; px < boundingBox.maxX; ++px)
		{
			for (int py{boundingBox.minY}; py < boundingBox.maxY; ++py)
			{
				//pixel values
				const int pixelIdx{ px + py * m_Width };
				const Vector2 pixelCoord{ static_cast<float>(px), static_cast<float>(py) };
				ColorRGB finalColor{ 0, 0, 0 };

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
					//interpolate depth values
					const float interpolatedDepth{
						m_vertices_screenSpace[i].position.z * weights[0]
						+ m_vertices_screenSpace[i + 1].position.z * weights[1]
						+ m_vertices_screenSpace[i + 2].position.z * weights[2] };


					if (interpolatedDepth < m_pDepthBufferPixels[pixelIdx])
					{
						m_pDepthBufferPixels[pixelIdx] = interpolatedDepth;

						//color pixel according to vertex colors
						finalColor +=
							m_vertices_screenSpace[i].color * weights[0]
							+ m_vertices_screenSpace[i + 1].color * weights[1]
							+ m_vertices_screenSpace[i + 2].color * weights[2];


						//Update Color in Buffer
						finalColor.MaxToOne();

						m_pBackBufferPixels[px + (py * m_Width)] = SDL_MapRGB(m_pBackBuffer->format,
							static_cast<uint8_t>(finalColor.r * 255),
							static_cast<uint8_t>(finalColor.g * 255),
							static_cast<uint8_t>(finalColor.b * 255));
					}
				}

			}
		}
	}
}
#pragma endregion


void dae::Renderer::Render_W2_Part1()
{
	
	//meshes
	const Mesh meshList{
		{ //vertices
			{{-3,3,-2} },
			{{0,3,-2}},
			{{3,3,-2}},
			{{-3,0,-2}},
			{{0,0,-2}},
			{{3,0,-2}},
			{{-3,-3,-2}},
			{{0,-3,-2}},
			{{3,-3,-2}}
		},

		{ //indices
			3,0,4,
			0,1,4,
			4,1,5,
			1,2,5,
			6,3,7,
			3,4,7,
			7,4,8,
			4,5,8
		},

		PrimitiveTopology::TriangeList
	};
	const Mesh meshStrip{
		{ //vertices
			{{-3,3,-2}},
			{{0,3,-2}},
			{{3,3,-2}},
			{{-3,0,-2}},
			{{0,0,-2}},
			{{3,0,-2}},
			{{-3,-3,-2}},
			{{0,-3,-2}},
			{{3,-3,-2}}
		},

		{ //indices
			3,0,4,1,5,2,
			2,6,
			6,3,7,4,8,5
		},

		PrimitiveTopology::TriangleStrip
	};
	std::vector<Vertex> vertices_worldSpace{};
	ParseMesh(meshList, vertices_worldSpace);

	//depth buffer
	std::fill_n(m_pDepthBufferPixels, m_Width * m_Height, FLT_MAX);
	SDL_FillRect(m_pBackBuffer, NULL, SDL_MapRGB(m_pBackBuffer->format, 100, 100, 100));


	//world -> screen
	VertexTransformationFunction(vertices_worldSpace, m_vertices_screenSpace);

	//for every tri
	for (size_t i = 0; i < m_vertices_screenSpace.size(); i += 3)
	{
		//construct triangle
		const Vector2 vertices[3]{ 
			{m_vertices_screenSpace[i].position.x, m_vertices_screenSpace[i].position.y}, 
			{m_vertices_screenSpace[i + 1].position.x, m_vertices_screenSpace[i + 1].position.y},
			{m_vertices_screenSpace[i + 2].position.x, m_vertices_screenSpace[i + 2].position.y} };

		const BoundingBox boundingBox{ GenerateBoundingBox(vertices) };

		//every pixel
		for (int px{ boundingBox.minX }; px <= boundingBox.maxX; ++px)
		{
			for (int py{ boundingBox.minY }; py <= boundingBox.maxY; ++py)
			{
				//pixel values
				const int pixelIdx{ px + py * m_Width };
				const Vector2 pixelCoord{ static_cast<float>(px), static_cast<float>(py) };
				ColorRGB finalColor{ 0, 0, 0 };


				float weights[3]{};
				if (IsPointInTri(pixelCoord, vertices, weights))
				{
					//interpolate depth values
					const float interpolatedDepth{
						m_vertices_screenSpace[i].position.z * weights[0]
						+ m_vertices_screenSpace[i + 1].position.z * weights[1]
						+ m_vertices_screenSpace[i + 2].position.z * weights[2] };


					if (interpolatedDepth < m_pDepthBufferPixels[pixelIdx])
					{
						m_pDepthBufferPixels[pixelIdx] = interpolatedDepth;

						//color pixel according to vertex colors
						finalColor +=
							m_vertices_screenSpace[i].color * weights[0]
							+ m_vertices_screenSpace[i + 1].color * weights[1]
							+ m_vertices_screenSpace[i + 2].color * weights[2];


						//Update Color in Buffer
						finalColor.MaxToOne();

						m_pBackBufferPixels[px + (py * m_Width)] = SDL_MapRGB(m_pBackBuffer->format,
							static_cast<uint8_t>(finalColor.r * 255),
							static_cast<uint8_t>(finalColor.g * 255),
							static_cast<uint8_t>(finalColor.b * 255));
					}
				}
			}
		}
	}
}

void dae::Renderer::Render_W2_Part2()
{
	//meshes
	const Mesh meshList{
		{ //vertices
			//pos		//color		//uv
			{{-3,3,-2},		{},		{0,0}		},
			{{0,3,-2},		{},		{.5f,0}		},
			{{3,3,-2},		{},		{1,0}		},
			{{-3,0,-2},		{},		{0,.5f}		},
			{{0,0,-2},		{},		{.5f,.5f}	},
			{{3,0,-2},		{},		{1,.5f}		},
			{{-3,-3,-2},	{},		{0,1}		},
			{{0,-3,-2},		{},		{.5f,1}		},
			{{3,-3,-2},		{},		{1,1}		}
		},

		{ //indices
			3,0,4,
			0,1,4,
			4,1,5,
			1,2,5,
			6,3,7,
			3,4,7,
			7,4,8,
			4,5,8
		},

		PrimitiveTopology::TriangeList
	};
	const Mesh meshStrip{
		{ //vertices
			//pos		//color		//uv
			{{-3,3,-2},		{},		{0.f, 0.f}		},
			{{0,3,-2},		{},		{.5f, 0.f}		},
			{{3,3,-2},		{},		{1.f, 0.f}		},
			{{-3,0,-2},		{},		{0.f, .5f}		},
			{{0,0,-2},		{},		{.5f, .5f}		},
			{{3,0,-2},		{},		{1.f, .5f}		},
			{{-3,-3,-2},	{},		{0.f, 1.f}		},
			{{0,-3,-2},		{},		{.5f, 1.f}		},
			{{3,-3,-2},		{},		{1.f, 1.f}		}
		},

		{ //indices
			3,0,4,1,5,2,
			2,6,
			6,3,7,4,8,5
		},

		PrimitiveTopology::TriangleStrip
	};
	std::vector<Vertex> vertices_worldSpace{};

	ParseMesh(meshList, vertices_worldSpace);

	//depth buffer
	std::fill_n(m_pDepthBufferPixels, m_Width * m_Height, FLT_MAX);
	SDL_FillRect(m_pBackBuffer, NULL, SDL_MapRGB(m_pBackBuffer->format, 100, 100, 100));

	//world -> screen
	VertexTransformationFunction(vertices_worldSpace, m_vertices_screenSpace);

	//for every tri
	for (size_t i = 0; i < m_vertices_screenSpace.size(); i += 3)
	{
		//construct triangle
		const Vector2 vertices[3]{
			{m_vertices_screenSpace[i].position.x, m_vertices_screenSpace[i].position.y},
			{m_vertices_screenSpace[i + 1].position.x, m_vertices_screenSpace[i + 1].position.y},
			{m_vertices_screenSpace[i + 2].position.x, m_vertices_screenSpace[i + 2].position.y} };

		const BoundingBox boundingBox{ GenerateBoundingBox(vertices) };

		//every pixel
		for (int px{ boundingBox.minX }; px <= boundingBox.maxX; ++px)
		{
			for (int py{ boundingBox.minY }; py <= boundingBox.maxY; ++py)
			{
				//pixel values
				const int pixelIdx{ px + py * m_Width };
				const Vector2 pixelCoord{ static_cast<float>(px), static_cast<float>(py) };
				ColorRGB finalColor{ 0, 0, 0 };

				float weights[3]{};
				if (IsPointInTri(pixelCoord, vertices, weights))
				{
					//interpolate depth values
					const float interpolatedDepth
					{
						1 / ( 
						( (1/m_vertices_screenSpace[i].position.z)		* weights[0]) + 
						( (1/m_vertices_screenSpace[i + 1].position.z)	* weights[1]) + 
						( (1/m_vertices_screenSpace[i + 2].position.z)	* weights[2]))
					};

					if (interpolatedDepth < m_pDepthBufferPixels[pixelIdx])
					{
						m_pDepthBufferPixels[pixelIdx] = interpolatedDepth;

						const Vector2 uvInterpolated{
							(
							((m_vertices_screenSpace[i].uv / m_vertices_screenSpace[i].position.z)			* weights[0]) +
							((m_vertices_screenSpace[i + 1].uv / m_vertices_screenSpace[i + 1].position.z)	* weights[1]) +
							((m_vertices_screenSpace[i + 2].uv / m_vertices_screenSpace[i + 2].position.z)	* weights[2])
							) 
							* interpolatedDepth
						};

						//color pixel according to uv
						finalColor = m_pTexture->Sample(uvInterpolated);

						//Update Color in Buffer
						finalColor.MaxToOne();

						m_pBackBufferPixels[px + (py * m_Width)] = SDL_MapRGB(m_pBackBuffer->format,
							static_cast<uint8_t>(finalColor.r * 255),
							static_cast<uint8_t>(finalColor.g * 255),
							static_cast<uint8_t>(finalColor.b * 255));
					}
				}
			}
		}
	}
}

void dae::Renderer::Render_W2_Part2_2()
{
	//meshes
	std::vector<Mesh> meshes_world
	{
		Mesh {
			{ //vertices
				//pos		//color		//uv
				{{-3,3,-2},		{},		{0,0}		},
				{{0,3,-2},		{},		{.5f,0}		},
				{{3,3,-2},		{},		{1,0}		},
				{{-3,0,-2},		{},		{0,.5f}		},
				{{0,0,-2},		{},		{.5f,.5f}	},
				{{3,0,-2},		{},		{1,.5f}		},
				{{-3,-3,-2},	{},		{0,1}		},
				{{0,-3,-2},		{},		{.5f,1}		},
				{{3,-3,-2},		{},		{1,1}		}
			},

			{ //indices
				3,0,4,
				0,1,4,
				4,1,5,
				1,2,5,
				6,3,7,
				3,4,7,
				7,4,8,
				4,5,8
			},

			PrimitiveTopology::TriangeList
		},

		Mesh {
			{ //vertices
				//pos		//color		//uv
				{{-3,3,-2},		{},		{0.f, 0.f}		},
				{{0,3,-2},		{},		{.5f, 0.f}		},
				{{3,3,-2},		{},		{1.f, 0.f}		},
				{{-3,0,-2},		{},		{0.f, .5f}		},
				{{0,0,-2},		{},		{.5f, .5f}		},
				{{3,0,-2},		{},		{1.f, .5f}		},
				{{-3,-3,-2},	{},		{0.f, 1.f}		},
				{{0,-3,-2},		{},		{.5f, 1.f}		},
				{{3,-3,-2},		{},		{1.f, 1.f}		}
			},

			{ //indices
				3,0,4,1,5,2,
				2,6,
				6,3,7,4,8,5
			},

			PrimitiveTopology::TriangleStrip
		}
	};
	const uint32_t meshIdx{ 0 };

	//std::vector<Vertex> vertices_worldSpace{};
	//for (Mesh m : stripMeshes_world) {
	//	ParseMesh(m, vertices_worldSpace);
	//}

	//depth buffer
	std::fill_n(m_pDepthBufferPixels, m_Width * m_Height, FLT_MAX);
	SDL_FillRect(m_pBackBuffer, NULL, SDL_MapRGB(m_pBackBuffer->format, 100, 100, 100));

	//world -> screen
	VertexTransformationFunction(meshes_world[meshIdx].vertices, meshes_world[meshIdx].vertices_out);

	//for every tri
	for (size_t i = meshes_world[meshIdx].primitiveTopology == PrimitiveTopology::TriangeList ? 0 : 2; 
		i < meshes_world[meshIdx].indices.size(); ++i)
	{
		const Vertex_Out vertices[3]
		{
			meshes_world[meshIdx].vertices_out[meshes_world[meshIdx].indices[i]],
			meshes_world[meshIdx].vertices_out[meshes_world[meshIdx].indices[++i]],
			meshes_world[meshIdx].vertices_out[meshes_world[meshIdx].indices[++i]]
		};

		const Vector2 verticePositions[3]
		{
			{ vertices[0].position.x, vertices[0].position.y },
			{ vertices[1].position.x, vertices[1].position.y },
			{ vertices[2].position.x, vertices[2].position.y }
		};	  

		/*if(i % 2 != 0)
		{
			vertices[0] = listMeshes_world[0].vertices[stripMeshes_world[0].indices[i - 2]];
			verticePositions[0].x = vertices[0].position.x;
			verticePositions[0].y = vertices[0].position.y;

			if (i % 2 != 0)
			{
				vertices[1] = stripMeshes_world[0].vertices[stripMeshes_world[0].indices[i]];
				verticePositions[1].x = vertices[1].position.x;
				verticePositions[1].y = vertices[1].position.y;

				vertices[2] = stripMeshes_world[0].vertices[stripMeshes_world[0].indices[i - 1]];
				verticePositions[2].x = vertices[2].position.x;
				verticePositions[2].y = vertices[2].position.y;
			}
			else
			{
				vertices[2] = stripMeshes_world[0].vertices[stripMeshes_world[0].indices[i]];
				verticePositions[2].x = vertices[2].position.x;
				verticePositions[2].y = vertices[2].position.y;

				vertices[1] = stripMeshes_world[0].vertices[stripMeshes_world[0].indices[i - 1]];
				verticePositions[1].x = vertices[1].position.x;
				verticePositions[1].y = vertices[1].position.y;
			}
		}*/

		//construct triangle
		//const Vector2 vertices[3]{
		//	{m_vertices_screenSpace[i].position.x, m_vertices_screenSpace[i].position.y},
		//	{m_vertices_screenSpace[i + 1].position.x, m_vertices_screenSpace[i + 1].position.y},
		//	{m_vertices_screenSpace[i + 2].position.x, m_vertices_screenSpace[i + 2].position.y} };

		const BoundingBox boundingBox{ GenerateBoundingBox(verticePositions) };

		//every pixel
		for (int px{ boundingBox.minX }; px <= boundingBox.maxX; ++px)
		{
			for (int py{ boundingBox.minY }; py <= boundingBox.maxY; ++py)
			{
				//pixel values
				const int pixelIdx{ px + py * m_Width };
				const Vector2 pixelCoord{ static_cast<float>(px), static_cast<float>(py) };
				ColorRGB finalColor{ 0, 0, 0 };

				float weights[3]{};
				if (IsPointInTri(pixelCoord, verticePositions, weights))
				{
					//interpolate depth values
					const float interpolatedDepth{
						vertices[0].position.z * weights[0]
						+ vertices[1].position.z * weights[1]
						+ vertices[2].position.z * weights[2] };


					if (interpolatedDepth < m_pDepthBufferPixels[pixelIdx])
					{
						m_pDepthBufferPixels[pixelIdx] = interpolatedDepth;

						//color pixel according to uv
						finalColor +=
							m_pTexture->Sample(vertices[0].uv * weights[0])
							+ m_pTexture->Sample(vertices[1].uv * weights[1])
							+ m_pTexture->Sample(vertices[2].uv * weights[2]);

						//Update Color in Buffer
						finalColor.MaxToOne();

						m_pBackBufferPixels[px + (py * m_Width)] = SDL_MapRGB(m_pBackBuffer->format,
							static_cast<uint8_t>(finalColor.r * 255),
							static_cast<uint8_t>(finalColor.g * 255),
							static_cast<uint8_t>(finalColor.b * 255));
					}
				}
			}
		}
	}
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

void Renderer::VertexTransformationFunction(const std::vector<Vertex>& vertices_in, std::vector<Vertex_Out>& vertices_out) const
{
	float aspectRatio{ static_cast<float>(m_Width) / static_cast<float>(m_Height) };
	vertices_out.reserve(vertices_in.size());

	//Todo > W1 Projection Stage
	for (int i = 0; i < vertices_in.size(); ++i)
	{
		Vector3 transformedPos{ m_Camera.viewMatrix.TransformPoint(vertices_in[i].position) };
		//transform vertices to viewSpace
		vertices_out[i].position.x = transformedPos.x;
		vertices_out[i].position.y = transformedPos.y;
		vertices_out[i].position.z = transformedPos.z;


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


bool dae::Renderer::IsPointInTri(Vector2 P, const Vector2 vertexPositions[], float(&weights)[3]) const
{
	const Vector2 edges[3]{
		vertexPositions[1] - vertexPositions[0],
		vertexPositions[2] - vertexPositions[1],
		vertexPositions[0] - vertexPositions[2] };

	Vector3 pointToSide{};
	const float totalArea{ Vector2::Cross(edges[0], edges[1]) };

	for (int i = 0; i < 3; ++i)
	{
		pointToSide.x = P.x - vertexPositions[i].x;
		pointToSide.y = P.y - vertexPositions[i].y;

		const float cross{ edges[i].x * pointToSide.y - edges[i].y * pointToSide.x };
		weights[i] = cross / totalArea;
		if (cross < 0) return false;
	}

	return true;
}

BoundingBox dae::Renderer::GenerateBoundingBox(const Vector2 vertices[]) const
{ 
	BoundingBox boundingBox{};

	for (size_t i = 0; i < 3; i++)
	{
		boundingBox.minX = std::min(int(vertices[i].x), boundingBox.minX);
		boundingBox.minY = std::min(int(vertices[i].y), boundingBox.minY);

		boundingBox.maxX = std::max(int(vertices[i].x), boundingBox.maxX);
		boundingBox.maxY = std::max(int(vertices[i].y), boundingBox.maxY);
	}

	if (boundingBox.minX < 0) boundingBox.minX = 0;
	if (boundingBox.maxX > m_Width) boundingBox.maxX = m_Width;
	if (boundingBox.minY < 0) boundingBox.minY = 0;
	if (boundingBox.maxY > m_Height) boundingBox.maxY = m_Height;

	return boundingBox;
}


void dae::Renderer::ParseMesh(Mesh mesh, std::vector<Vertex>& vertices_out)
{
	switch (mesh.primitiveTopology)
	{
	case PrimitiveTopology::TriangeList:
		for (size_t i = 0; i < mesh.indices.size(); i++)
		{
			vertices_out.emplace_back(mesh.vertices[mesh.indices[i]]);
		}
		break;

	case PrimitiveTopology::TriangleStrip:
		for (size_t i = 2; i < mesh.indices.size(); ++i)
		{
			vertices_out.emplace_back(mesh.vertices[mesh.indices[i - 2]]);

			if (i % 2 != 0)
			{
				vertices_out.emplace_back(mesh.vertices[mesh.indices[i]]);
				vertices_out.emplace_back(mesh.vertices[mesh.indices[i - 1]]);
			}
			else
			{
				vertices_out.emplace_back(mesh.vertices[mesh.indices[i - 1]]);
				vertices_out.emplace_back(mesh.vertices[mesh.indices[i]]);
			}
		}
		break;
	}
}





