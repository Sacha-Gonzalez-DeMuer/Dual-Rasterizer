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
	m_Camera.Initialize(60.f, { .0f, 5.f, -30.f }, 
		(static_cast<float>(m_Width) / static_cast<float>(m_Height)));

	m_pTexture = Texture::LoadFromFile("Resources/uv_grid_2.png");

	InitializeMesh();
}

Renderer::~Renderer()
{
	delete[] m_pDepthBufferPixels;

	delete m_pTexture;
	m_pTexture = nullptr;
}

void dae::Renderer::InitializeMesh()
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

	//m_Mesh = meshList;

	Utils::ParseOBJ("Resources/tuktuk.obj", m_Mesh.vertices, m_Mesh.indices, false);
	m_Mesh.primitiveTopology = PrimitiveTopology::TriangleStrip;
}


void Renderer::Update(Timer* pTimer)
{
	m_Camera.Update(pTimer);
	UpdateWorldViewProjectionMatrix(m_Mesh.worldMatrix, m_Camera.viewMatrix, m_Camera.projectionMatrix);
}

void Renderer::Render()
{
	//@START
	//Lock BackBuffer
	SDL_LockSurface(m_pBackBuffer);

	RenderLoop();

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


void dae::Renderer::RenderLoop()
{
	//depth buffer
	std::fill_n(m_pDepthBufferPixels, m_Width * m_Height, FLT_MAX);
	SDL_FillRect(m_pBackBuffer, NULL, SDL_MapRGB(m_pBackBuffer->format, 100, 100, 100));

	//world -> NDC
	VertexTransformationFunction(m_Mesh);
	std::vector<Vertex_Out>& vertices_NDC{ m_Mesh.vertices_out }; //in NDC space

	Triangle t{};
	//for every tri
	for (size_t i = 0; i < vertices_NDC.size(); i += 3)
	{
		//Construct Triangle
		t.vertices[0] = vertices_NDC[i];
		t.vertices[1] = vertices_NDC[i + 1];
		t.vertices[2] = vertices_NDC[i + 2];
		
		if (!IsTriangleInFrustum(t)) continue;

		for (size_t j = 0; j < 3; j++)
		{
			//NDC space -> screen space
			t.vertices[j].position.x = (t.vertices[j].position.x + 1) / 2 * static_cast<float>(m_Width);
			t.vertices[j].position.y = (1 - t.vertices[j].position.y) / 2 * static_cast<float>(m_Height);
		}
		
		const BoundingBox boundingBox{ GenerateBoundingBox(t) };
		
		PixelLoop(t, boundingBox);
	}
}

void dae::Renderer::PixelLoop(const Triangle& t, const BoundingBox& bb)
{
	for (int px{ bb.minX }; px <= bb.maxX; ++px)
	{
		for (int py{ bb.minY }; py <= bb.maxY; ++py)
		{
			//pixel values
			const int pixelIdx{ px + py * m_Width };
			const Vector2 pixelCoord{ static_cast<float>(px), static_cast<float>(py) };
			ColorRGB finalColor{ 0, 0, 0 };

			float weights[3]{};
			if (IsPointInTri(pixelCoord, t, weights))
			{
				const float interpolatedDepthZ
				{
					1 / (
					((1 / t.vertices[0].position.z) * weights[1]) +
					((1 / t.vertices[1].position.z) * weights[2]) +
					((1 / t.vertices[2].position.z) * weights[0]))
				};

				if (interpolatedDepthZ < m_pDepthBufferPixels[pixelIdx])
				{
					m_pDepthBufferPixels[pixelIdx] = interpolatedDepthZ;

					//interpolate depth values
					const float interpolatedDepthW
					{
						1 / (
						((1 / t.vertices[0].position.w) * weights[1]) +
						((1 / t.vertices[1].position.w) * weights[2]) +
						((1 / t.vertices[2].position.w) * weights[0]))
					};
					const Vector2 uvInterpolated{
						(
						((t.vertices[0].uv / t.vertices[0].position.w) * weights[1]) +
						((t.vertices[1].uv / t.vertices[1].position.w) * weights[2]) +
						((t.vertices[2].uv / t.vertices[2].position.w) * weights[0])
						)
						* interpolatedDepthW
					};

					switch (m_CurrentRenderMode)
					{
					case dae::RenderMode::FinalColor:
						//color pixel according to uv
						finalColor = m_pTexture->Sample(uvInterpolated);
						break;
					case dae::RenderMode::DepthBuffer:
						const float greyVal{ Remap(m_pDepthBufferPixels[pixelIdx], .985f, 1.f)};
						finalColor = { greyVal, greyVal, greyVal };
						break;
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
	}
}


//Transforms vertices in mesh to NDC space and stores them in the meshes vertex_out vector
void Renderer::VertexTransformationFunction(Mesh& mesh)
{
	// Parse the mesh and store the vertices in world space
	std::vector<Vertex> vertices_worldSpace{};
	ParseMesh(mesh, vertices_worldSpace);

	mesh.vertices_out.resize(vertices_worldSpace.size());

	// Transform the vertices from world space to clip space
	for (int i = 0; i < vertices_worldSpace.size(); ++i)
	{
		mesh.vertices_out[i].position = { vertices_worldSpace[i].position.x, vertices_worldSpace[i].position.y, vertices_worldSpace[i].position.z, 1 };

		//world space -> clip space
		mesh.vertices_out[i].position = m_WorldViewProjectionMatrix.TransformPoint(mesh.vertices_out[i].position);

		//apply perspective divide => clip space -> NDC space
		mesh.vertices_out[i].position.x /= mesh.vertices_out[i].position.w;
		mesh.vertices_out[i].position.y /= mesh.vertices_out[i].position.w;
		mesh.vertices_out[i].position.z /= mesh.vertices_out[i].position.w;

		// Copy the UV coordinates from the input vertex
		mesh.vertices_out[i].uv = vertices_worldSpace[i].uv;
	}
}

#pragma endregion

bool dae::Renderer::IsVertexInFrustum(const Vertex_Out& v)
{
	if (v.position.x < -1 || v.position.x > 1) return false;
	if (v.position.y < -1 || v.position.y > 1) return false;
	if (v.position.z < 0 || v.position.z > 1) return false;

	return true;
}

bool dae::Renderer::IsTriangleInFrustum(const Triangle& t)
{
	if (!IsVertexInFrustum(t.vertices[0])) return false;
	if (!IsVertexInFrustum(t.vertices[1])) return false;
	if (!IsVertexInFrustum(t.vertices[2])) return false;

	return true;
}

//checks if point is in 2D triangle and stores barycentric weights in given array
bool dae::Renderer::IsPointInTri(Vector2 P, const Triangle& t, float(&weights)[3]) const
{
	const Vector2 positions[3]
	{
		{t.vertices[0].position.x, t.vertices[0].position.y},
		{t.vertices[1].position.x, t.vertices[1].position.y},
		{t.vertices[2].position.x, t.vertices[2].position.y}
	};

	const Vector2 edges[3]
	{
		positions[1] - positions[0],
		positions[2] - positions[1],
		positions[0] - positions[2]
	};

	Vector3 pointToSide{};
	const float totalArea{ Vector2::Cross(edges[0], edges[1]) };

	for (int i = 0; i < 3; ++i)
	{
		pointToSide.x = P.x - positions[i].x;
		pointToSide.y = P.y - positions[i].y;

		const float cross{ edges[i].x * pointToSide.y - edges[i].y * pointToSide.x };
		weights[i] = cross / totalArea;
		if (cross < 0) return false;
	}

	return true;
}

BoundingBox dae::Renderer::GenerateBoundingBox(const Triangle t) const
{
	BoundingBox boundingBox{};

	for (size_t i = 0; i < 3; i++)
	{
		boundingBox.minX = std::min(static_cast<int>(t.vertices[i].position.x), boundingBox.minX);
		boundingBox.minY = std::min(static_cast<int>(t.vertices[i].position.y), boundingBox.minY);

		boundingBox.maxX = std::max(static_cast<int>(t.vertices[i].position.x), boundingBox.maxX);
		boundingBox.maxY = std::max(static_cast<int>(t.vertices[i].position.y), boundingBox.maxY);
	}

	if (boundingBox.minX < 0) boundingBox.minX = 0;
	if (boundingBox.maxX > m_Width) boundingBox.maxX = m_Width;
	if (boundingBox.minY < 0) boundingBox.minY = 0;
	if (boundingBox.maxY > m_Height) boundingBox.maxY = m_Height;

	return boundingBox;
}

float dae::Renderer::Remap(float value, float rangeMin, float rangeMax)
{
	return  (rangeMax - rangeMin) / (rangeMax - value);
}

void dae::Renderer::UpdateWorldViewProjectionMatrix(const Matrix& worldMatrix, const Matrix& viewMatrix, const Matrix& projectionMatrix)
{
	//if any transformation matrix changes, update worldViewProjectionMatrix
	if (m_WorldMatrix != worldMatrix || m_ViewMatrix != viewMatrix || m_ProjectionMatrix != projectionMatrix)
	{
		m_WorldMatrix = worldMatrix;
		m_ViewMatrix = viewMatrix;
		m_ProjectionMatrix = projectionMatrix;
		m_WorldViewProjectionMatrix = m_WorldMatrix * m_ViewMatrix * m_ProjectionMatrix;
	}
}

void dae::Renderer::ParseMesh(Mesh mesh, std::vector<Vertex>& vertices_out)
{
	switch (mesh.primitiveTopology)
	{
	case PrimitiveTopology::TriangeList:
		for (size_t i = 0; i < mesh.indices.size(); ++i)
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

void dae::Renderer::CycleRenderMode()
{
	int current = static_cast<int>(m_CurrentRenderMode);
	++current;
	current = current % 2;
	m_CurrentRenderMode = static_cast<RenderMode>(current);
}



