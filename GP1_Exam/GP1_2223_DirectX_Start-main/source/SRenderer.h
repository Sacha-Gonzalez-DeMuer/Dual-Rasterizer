#pragma once
#include "pch.h"
#include "Renderer.h"

using namespace dae;

class Timer;
class Mesh;

class SRenderer : virtual public Renderer
{
public:
	explicit SRenderer(SDL_Window* pWindow);
	virtual ~SRenderer() override;
	virtual void Update(dae::Timer* pTimer) override;
	virtual void Render() override;

protected:

	SDL_Surface* m_pFrontBuffer{ nullptr };
	SDL_Surface* m_pBackBuffer{ nullptr };
	uint32_t* m_pBackBufferPixels{};

	float* m_pDepthBufferPixels{};


	RenderMode m_CurrentRenderMode{ RenderMode::FinalColor };
	ShadingMode m_CurrentShadingMode{ ShadingMode::Combined };
	bool m_NormalToggled{ true };
	bool m_RotationToggled{ true };

	void RenderLoop();
	void PixelLoop(const Triangle& t, const BoundingBox& bb, std::shared_ptr<Mesh> mesh);
	Vertex_Out GetInterpolatedVert(const Triangle& t);

	bool IsVertexInFrustum(const Vertex_Out& v) const;
	bool IsTriangleInFrustum(const Triangle& t) const;
	bool IsPointInTri(const Vector2& P, const Triangle& t, float(&weights)[3]) const;
	BoundingBox GenerateBoundingBox(const Triangle t) const;
	float Remap(float value, float rangeMin, float rangeMax);
};

