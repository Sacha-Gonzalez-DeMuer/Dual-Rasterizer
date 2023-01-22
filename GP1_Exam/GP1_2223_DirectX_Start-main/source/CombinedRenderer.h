#pragma once
#include "SRenderer.h"
#include "DXRenderer.h"

class CombinedRenderer final : public SRenderer, DXRenderer
{
public:
	explicit CombinedRenderer(SDL_Window* pWindow);
	void Update(dae::Timer* pTimer);
	void Render();

	void ToggleGPU();
	void ToggleRotation();
	void ToggleNormalMap();
	void ToggleDepthBuffer();
	void ToggleBoundingBox();

	void CycleCullMode();
	void CycleSampleState();
	void CycleShadingMode();

	void BoostMovementSpeed();
private:
	bool m_UseGPU;
	bool m_RotateMesh;
	bool m_UseNormalMaps;
	bool m_VisualizeDepthBuffer;

	CullMode m_CullMode;
	SamplerState m_SampleState;
	ShadingMode m_ShadingMode;
};

