#pragma once
#include "pch.h"
#include "Renderer.h"

struct Vertex;
class Timer;

class SRenderer : public Renderer
{
public:
	explicit SRenderer(SDL_Window* pWindow, std::shared_ptr<Scene> sceneToLoad);


	virtual void Render() override;
	virtual void Update(Timer* pTimer) override;

private:
	SDL_Window* m_pWindow{};

	SDL_Surface* m_pFrontBuffer{ nullptr };
	SDL_Surface* m_pBackBuffer{ nullptr };
	uint32_t* m_pBackBufferPixels{};

	float* m_pDepthBufferPixels{};


	//Function that transforms the vertices from the mesh from World space to Screen space
	void VertexTransformationFunction(const std::vector<Vertex>& vertices_in, std::vector<Vertex>& vertices_out) const; //W1 Version
};

