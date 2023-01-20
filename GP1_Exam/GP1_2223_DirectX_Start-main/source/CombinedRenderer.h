#pragma once
#include "SRenderer.h"
#include "DXRenderer.h"

class CombinedRenderer final : public SRenderer, DXRenderer
{
public:
	explicit CombinedRenderer(SDL_Window* pWindow);
	~CombinedRenderer();


	void Update(dae::Timer* pTimer);
	void Render();
private:
	bool m_UseGPU;
};

