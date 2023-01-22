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
	void CycleCullMode();
private:
	bool m_UseGPU;
	bool m_RotateMesh;

	CullMode m_CullMode;
};

