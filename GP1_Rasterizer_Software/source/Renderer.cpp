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
#include "BRDFs.h"

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
	m_Camera.Initialize(60.f, { .0f, .0f, -40.f },
		(static_cast<float>(m_Width) / static_cast<float>(m_Height)));

	m_pTexture = Texture::LoadFromFile("Resources/vehicle_diffuse.png");
	m_pNormalMap = Texture::LoadFromFile("Resources/vehicle_normal.png");
	m_pSpecularColor = Texture::LoadFromFile("Resources/vehicle_specular.png");
	m_pPhongExponent = Texture::LoadFromFile("Resources/vehicle_gloss.png");

	InitializeMesh();
}

Renderer::~Renderer()
{
	delete[] m_pDepthBufferPixels;

	delete m_pTexture;
	m_pTexture = nullptr;

	delete m_pNormalMap;
	m_pNormalMap = nullptr;
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
	//ParseMesh(m_Mesh); 

	Utils::ParseOBJ("Resources/vehicle.obj", m_Mesh.vertices, m_Mesh.indices, false);
	m_Mesh.vertices_out.resize(m_Mesh.vertices.size());
}

void Renderer::Update(Timer* pTimer)
{
	m_Camera.Update(pTimer);

	//Rotate mesh
	if (m_RotationToggled)
	{
		m_Mesh.yawPitchRoll.x += pTimer->GetElapsed();
		m_Mesh.worldMatrix = Matrix::CreateRotationY(m_Mesh.yawPitchRoll.x);
	}

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

	//for every tri
	for (size_t i = 0; i < m_Mesh.vertices_out.size(); i += 3)
	{
		//Construct Triangle
		Triangle t{ { m_Mesh.vertices_out[i], m_Mesh.vertices_out[i + 1], m_Mesh.vertices_out[i + 2] } };
		
		if (!IsTriangleInFrustum(t)) continue; //frustum culling

		for (size_t j = 0; j < 3; j++)
		{
			//NDC space -> screen/raster space
			t.vertices[j].position.x = (t.vertices[j].position.x + 1) / 2 * static_cast<float>(m_Width);
			t.vertices[j].position.y = (1 - t.vertices[j].position.y) / 2 * static_cast<float>(m_Height);
		}

		t.recipTotalArea = 1 / Vector2::Cross(t.GetVector2Pos(0) - t.GetVector2Pos(1), t.GetVector2Pos(0) - t.GetVector2Pos(2));
		t.edges[0] = t.GetVector2Pos(1) - t.GetVector2Pos(0);
		t.edges[1] = t.GetVector2Pos(2) - t.GetVector2Pos(1);
		t.edges[2] = t.GetVector2Pos(0) - t.GetVector2Pos(2);

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
			const int pixelIdx{ px + (py * m_Width) };
			const Vector2 pixelCoord{ static_cast<float>(px), static_cast<float>(py) };
			ColorRGB finalColor{ 0, 0, 0 };

			int pointsInTriTested{ 0 };

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

					switch (m_CurrentRenderMode)
					{
					case dae::RenderMode::FinalColor:
						finalColor = PixelShading(interpolatedVertex);
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
				};
			}
		}
	}
}

//v's values should be interpolated
ColorRGB dae::Renderer::PixelShading(const Vertex_Out& v)
{
	ColorRGB finalColor{};
	const Vector3 lightDirection { .577f, -.577f, .577f };
	constexpr float lightIntensity{ 7.f };

	constexpr float specularShininess{ 25.f };
	const float diffuseReflectance{ 1.f }; 
	const float specular{ m_pSpecularColor->Sample(v.uv).r };
	const float phongExponent{ m_pPhongExponent->Sample(v.uv).g * specularShininess };
	Vector3 viewDirection{  m_Camera.invViewMatrix.TransformVector(m_Camera.forward).Normalized() };

	//create tangent space transformation matrix
	const Vector3 binormal{ Vector3::Cross(v.normal, v.tangent ).Normalized() };
	const Matrix tangentSpace{ -v.tangent, binormal, v.normal, Vector3::Zero };

	//sample normal
	const ColorRGB sampledNormal{ (2.f * m_pNormalMap->Sample(v.uv) - ColorRGB(1,1,1)) }; //to [-1, 1]
	const Vector3 vSampledNormal{ Vector3(sampledNormal.r, sampledNormal.g, sampledNormal.b) };
	const Vector3 tangentSpaceSampledNormal { tangentSpace.TransformVector(vSampledNormal).Normalized() };
	
	//choose which normal to use
	Vector3 usedNormal{};
	if (m_NormalToggled)
		usedNormal = tangentSpaceSampledNormal;
	else
		usedNormal = v.normal;

	const float observedArea{ std::max(0.f, Vector3::Dot(usedNormal, -lightDirection)) };

	//BRDFs
	const ColorRGB lambertDiffuse{ BRDF::Lambert(diffuseReflectance, m_pTexture->Sample(v.uv)) * lightIntensity };
	const ColorRGB phongSpecular{ BRDF::Phong(specular, phongExponent, lightDirection, -viewDirection, usedNormal)  };

	constexpr ColorRGB ambient{ .025f, .025f, .025f };

	switch (m_CurrentShadingMode)
	{
		case ShadingMode::ObservedArea:
			finalColor += ColorRGB(1, 1, 1) * observedArea;
			break;
		case ShadingMode::Diffuse:
			finalColor += lambertDiffuse * observedArea;
			break;
		case ShadingMode::Specular:
			finalColor += ColorRGB(1,1,1) * phongSpecular * observedArea;
			break;
		default:
		case ShadingMode::Combined:
			finalColor += lambertDiffuse * observedArea + phongSpecular + ambient;
	}

	finalColor.MaxToOne();

	return finalColor;
}

//Transforms vertices in mesh to NDC space and stores them in the meshes vertex_out vector
void Renderer::VertexTransformationFunction(Mesh& mesh)
{
	// Transform the vertices from world space to clip space
	for (int i = 0; i < mesh.vertices.size(); ++i)
	{
		mesh.vertices_out[i].position = { mesh.vertices[i].position.x, mesh.vertices[i].position.y, mesh.vertices[i].position.z, 1 };

		//transform normals & tangents from model space to world space
		Matrix worldMatrix3x3{ mesh.worldMatrix.GetAxisX(), mesh.worldMatrix.GetAxisY(), mesh.worldMatrix.GetAxisZ(), Vector3::Zero };
		mesh.vertices_out[i].normal  = worldMatrix3x3.TransformVector(mesh.vertices[i].normal);
		mesh.vertices_out[i].tangent = worldMatrix3x3.TransformVector(mesh.vertices[i].tangent);

		//position to clip space
		mesh.vertices_out[i].position = m_WorldViewProjectionMatrix.TransformPoint(mesh.vertices_out[i].position);

		const float recipW{ 1 / mesh.vertices_out[i].position.w };

		//apply perspective divide => clip space -> NDC space
		mesh.vertices_out[i].position.x *= recipW;
		mesh.vertices_out[i].position.y *= recipW;
		mesh.vertices_out[i].position.z *= recipW;

		// Copy the UV coordinates from the input vertex
		mesh.vertices_out[i].uv = mesh.vertices[i].uv;
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
bool dae::Renderer::IsPointInTri(const Vector2& P, const Triangle& t, float(&weights)[3]) const
{	
	for (int i = 0; i < 3; ++i)
	{
		const Vector2 pointToSide{ P - t.GetVector2Pos(i) };
		const float cross{ t.edges[i].x * pointToSide.y - t.edges[i].y * pointToSide.x };
		weights[i] = cross * t.recipTotalArea;
		if (cross < 0) return false;
	}

	return true;
}

BoundingBox dae::Renderer::GenerateBoundingBox(const Triangle t) const
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

float dae::Renderer::Remap(float value, float rangeMin, float rangeMax)
{
	return (value - rangeMin) / (rangeMax - rangeMin);
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

void dae::Renderer::ParseMesh(Mesh& mesh)
{
	std::vector<Vertex> tmp{};

	switch (mesh.primitiveTopology)
	{
	case PrimitiveTopology::TriangeList:
		for (size_t i = 0; i < mesh.indices.size(); ++i)
		{
			tmp.emplace_back(mesh.vertices[mesh.indices[i]]);
		}
		break;

	case PrimitiveTopology::TriangleStrip:
		for (size_t i = 2; i < mesh.indices.size(); ++i)
		{
			tmp.emplace_back(mesh.vertices[mesh.indices[i - 2]]);

			if (i % 2 != 0)
			{
				tmp.emplace_back(mesh.vertices[mesh.indices[i]]);
				tmp.emplace_back(mesh.vertices[mesh.indices[i - 1]]);
			}
			else
			{
				tmp.emplace_back(mesh.vertices[mesh.indices[i - 1]]);
				tmp.emplace_back(mesh.vertices[mesh.indices[i]]);
			}
		}
		break;
	}

	mesh.vertices = tmp;
}

void dae::Renderer::CycleRenderMode()
{
	int current = static_cast<int>(m_CurrentRenderMode);
	++current;
	current = current % static_cast<int>(RenderMode::Size);
	m_CurrentRenderMode = static_cast<RenderMode>(current);
}

void dae::Renderer::CycleShadingMode()
{
	int current = static_cast<int>(m_CurrentShadingMode);
	++current;
	current = current % static_cast<int>(ShadingMode::Size);
	m_CurrentShadingMode = static_cast<ShadingMode>(current);
}

void dae::Renderer::ToggleNormalMap()
{
	m_NormalToggled = !m_NormalToggled;
}

void dae::Renderer::ToggleRotation()
{
	m_RotationToggled = !m_RotationToggled;
}



