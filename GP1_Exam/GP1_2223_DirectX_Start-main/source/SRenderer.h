#pragma once
#include "pch.h"
#include "Renderer.h"

class Timer;

class SRenderer : virtual public Renderer
{
public:
	explicit SRenderer(SDL_Window* pWindow);
	virtual ~SRenderer() override;
	virtual void Update(dae::Timer* pTimer) override;
	virtual void Render() override;

protected:
	SDL_Window* m_pWindow{};

	SDL_Surface* m_pFrontBuffer{ nullptr };
	SDL_Surface* m_pBackBuffer{ nullptr };
	uint32_t* m_pBackBufferPixels{};

	float* m_pDepthBufferPixels{};


	//Function that transforms the vertices from the mesh from World space to Screen space
	void VertexTransformationFunction(const std::vector<dae::Vertex>& vertices_in, std::vector<dae::Vertex>& vertices_out) const; //W1 Version
};

