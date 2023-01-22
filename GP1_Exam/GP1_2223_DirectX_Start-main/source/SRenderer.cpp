#include "pch.h"
#include "SRenderer.h"
#include "Mesh.h"
#include "Scene.h"
#include "VehicleScene.h"


void printPixelColors(SDL_Surface* surface) {
	for (int y = 0; y < surface->h; y++) {
		for (int x = 0; x < surface->w; x++) {
			Uint32 pixel = ((Uint32*)surface->pixels)[y * surface->w + x];
			Uint8 r, g, b, a;
			SDL_GetRGBA(pixel, surface->format, &r, &g, &b, &a);
			std::cout << "Pixel at (" << x << ", " << y << ") has color: R=" << (int)r << " G=" << (int)g << " B=" << (int)b << " A=" << (int)a << std::endl;
		}
	}
}


SRenderer::SRenderer(SDL_Window* pWindow)
	: Renderer(pWindow)
	, m_CullMode{CullMode::Backface}
{
	//Create Buffers
	m_pFrontBuffer = SDL_GetWindowSurface(pWindow);
	m_pBackBuffer = SDL_CreateRGBSurface(0, m_Width, m_Height, 32, 0, 0, 0, 0);

	m_pBackBufferPixels = (uint32_t*)m_pBackBuffer->pixels;
	m_pDepthBufferPixels = new float[m_Width * m_Height];
}

SRenderer::~SRenderer()
{
	if(m_pDepthBufferPixels)
		delete[] m_pDepthBufferPixels;


	if (m_pFrontBuffer)
	{
		SDL_FreeSurface(m_pFrontBuffer);
		m_pFrontBuffer = nullptr;
	}

	if (m_pBackBuffer)
	{
		SDL_FreeSurface(m_pBackBuffer);
		m_pBackBuffer = nullptr;
	}
}

void SRenderer::Render()
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

void SRenderer::Update(dae::Timer* pTimer)
{
	m_pScene->Update(pTimer);
}


void SRenderer::SetDepthBufferVisualization(bool visualize)
{
	m_VisualizeDepthBuffer = visualize;
}

void SRenderer::SetBoundingBoxVisualization(bool visualize)
{
	m_VisualizeBoundingBox = visualize;
}

void SRenderer::RenderLoop()
{
	//depth buffer
	std::fill_n(m_pDepthBufferPixels, m_Width * m_Height, FLT_MAX);

	ColorRGB backColor{ m_ClearColor ? BACKGROUND_SOFTWARE : BACKGROUND_CLEAR };
	SDL_FillRect(m_pBackBuffer, NULL, SDL_MapRGB(m_pBackBuffer->format, static_cast<int>(backColor.r * 255), static_cast<int>(backColor.g * 255), static_cast<int>(backColor.b * 255)));

	//for every mesh
	for (auto& mesh : m_pScene->GetMeshes())
	{
		if (!mesh->SInitialized()) continue;

		UpdateWorldViewProjectionMatrix(mesh->GetWorldMatrix(), m_pScene->GetCamera().GetViewMatrix(), m_pScene->GetCamera().GetProjectionMatrix());

		mesh->TriangleTransformationFunction(m_WorldViewProjectionMatrix);

		// for every tri
		for (auto& t : mesh->GetTriangles())
		{
			if (!IsTriangleInFrustum(t)) continue;

			for (size_t j{ 0 }; j < 3; ++j)
			{
				//NDC space -> screen/raster space
				t.vertices[j].position.x = (t.vertices[j].position.x + 1) / 2 * static_cast<float>(m_Width);
				t.vertices[j].position.y = (1 - t.vertices[j].position.y) / 2 * static_cast<float>(m_Height);
			}

			t.CalculateEdges();
			t.recipTotalArea = 1 / t.GetArea();

			const BoundingBox boundingBox{ GenerateBoundingBox(t) };
			PixelLoop(t, boundingBox, mesh);
		}
	}
}

void SRenderer::PixelLoop(const Triangle& t, const BoundingBox& bb, std::shared_ptr<Mesh> mesh)
{
	for (int px{ bb.minX }; px <= bb.maxX; ++px)
	{
		for (int py{ bb.minY }; py <= bb.maxY; ++py)
		{

			if (m_VisualizeBoundingBox)
			{
				//Update Color in Buffer
				
				m_pBackBufferPixels[px + (py * m_Width)] = SDL_MapRGB(m_pBackBuffer->format,
					static_cast<uint8_t>(255),
					static_cast<uint8_t>(255),
					static_cast<uint8_t>(255));
			}
			else {

				//pixel values
				const int pixelIdx{ px + (py * m_Width) };
				const Vector2 pixelCoord{ static_cast<float>(px), static_cast<float>(py) };
				ColorRGB finalColor{ 0, 0, 0 };

				float weights[3]{};
				if (IsPointInTri(pixelCoord, t, weights))
				{
					const float interpolatedDepthZ{ t.GetInterpolatedZ(weights) };

					if (interpolatedDepthZ > 0.f && interpolatedDepthZ < 1.f && interpolatedDepthZ < m_pDepthBufferPixels[pixelIdx])
					{
						m_pDepthBufferPixels[pixelIdx] = interpolatedDepthZ;

						const float interpolatedDepthW{ t.GetInterpolatedW(weights) };

						const float recipW0{ 1 / t.vertices[0].position.w };
						const float recipW1{ 1 / t.vertices[1].position.w };
						const float recipW2{ 1 / t.vertices[2].position.w };

						const Vector2 uvInterpolated{
						(
						((t.vertices[0].uv * recipW0) * weights[1]) +
						((t.vertices[1].uv * recipW1) * weights[2]) +
						((t.vertices[2].uv * recipW2) * weights[0])
						)
						* interpolatedDepthW
					};
						const Vector3 normalInterpolated{
							(
							((t.vertices[0].normal * recipW0) * weights[1]) +
							((t.vertices[1].normal * recipW1) * weights[2]) +
							((t.vertices[2].normal * recipW2) * weights[0])
							)
							* interpolatedDepthW
						};
						const Vector3 tangentInterpolated{
							(
							((t.vertices[0].tangent * recipW0) * weights[1]) +
							((t.vertices[1].tangent * recipW1) * weights[2]) +
							((t.vertices[2].tangent * recipW2) * weights[0])
							)
							* interpolatedDepthW
						};

						Vertex_Out interpolatedVertex{};	
						interpolatedVertex.normal = normalInterpolated.Normalized();
						interpolatedVertex.uv = uvInterpolated;
						interpolatedVertex.tangent = tangentInterpolated.Normalized();
						const float interX{
						1 / (
							((1 / t.vertices[0].position.x) * weights[1]) +
							((1 / t.vertices[1].position.x) * weights[2]) +
							((1 / t.vertices[2].position.x) * weights[0]))
					};
						const float interY{
						1 / (
						((1 / t.vertices[0].position.y) * weights[1]) +
						((1 / t.vertices[1].position.y) * weights[2]) +
						((1 / t.vertices[2].position.y) * weights[0]))
					};
						interpolatedVertex.position = { interX, interY, interpolatedDepthZ, interpolatedDepthW };

						if (m_VisualizeDepthBuffer)
						{
							const float greyVal{ Remap(m_pDepthBufferPixels[pixelIdx], .985f, 1.f) };
							finalColor = { greyVal, greyVal, greyVal };
						}
						else
						{
							finalColor = mesh->PixelShading(interpolatedVertex, m_pScene->GetCamera());
						}

						//Update Color in Buffer
						finalColor.MaxToOne();
						m_pBackBufferPixels[px + (py * m_Width)] = SDL_MapRGB(m_pBackBuffer->format,
							static_cast<uint8_t>(finalColor.r * 255),
							static_cast<uint8_t>(finalColor.g * 255),
							static_cast<uint8_t>(finalColor.b * 255));
					};
				}
			}
		}
	}
}

bool SRenderer::IsVertexInFrustum(const Vertex_Out& v) const
{
	if (v.position.x < -1 || v.position.x > 1) return false;
	if (v.position.y < -1 || v.position.y > 1) return false;
	if (v.position.z < 0 || v.position.z > 1) return false;

	return true;
}

bool SRenderer::IsTriangleInFrustum(const Triangle& t) const
{
	if (!IsVertexInFrustum(t.vertices[0])) return false;
	if (!IsVertexInFrustum(t.vertices[1])) return false;
	if (!IsVertexInFrustum(t.vertices[2])) return false;

	return true;
}

bool SRenderer::IsPointInTri(const Vector2& P, const Triangle& t, float(&weights)[3]) const
{
	for (int i = 0; i < 3; ++i)
	{
		const Vector2 pointToSide{ P - t.GetVector2Pos(i) };
		const float cross{ t.edges[i].x * pointToSide.y - t.edges[i].y * pointToSide.x };
		weights[i] = cross * t.recipTotalArea;

		switch (m_CullMode)
		{
			case CullMode::None:
			case CullMode::Backface:
				if (cross < 0) return false;
				break;

			case CullMode::Frontface:
				if (cross > 0) return false;
				break;
		}
	}

	return true;
}

BoundingBox SRenderer::GenerateBoundingBox(const Triangle t) const
{
	BoundingBox boundingBox{};
	boundingBox.minX = INT_MAX;
	boundingBox.minY = INT_MAX;

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

float SRenderer::Remap(float value, float rangeMin, float rangeMax)
{
	return (value - rangeMin) / (rangeMax - rangeMin);
}

void SRenderer::SetCullMode(CullMode mode)
{
	m_CullMode = mode;
}
