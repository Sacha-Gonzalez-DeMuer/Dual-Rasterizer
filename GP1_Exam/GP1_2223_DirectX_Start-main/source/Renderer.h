#pragma once

#include <cstdint>
#include <vector>
#include "Timer.h"
class Scene;

using namespace dae;

class Renderer
{
public:
	Renderer() = delete;
	Renderer(SDL_Window* pWindow);
	virtual ~Renderer();

	Renderer(const Renderer&) = delete;
	Renderer(Renderer&&) noexcept = delete;
	Renderer& operator=(const Renderer&) = delete;\
	Renderer& operator=(Renderer&&) noexcept = delete;

	virtual void Update(dae::Timer* pTimer) = 0;
	virtual void Render() = 0;
	
	void SetSceneToRender(std::shared_ptr<Scene> scene) { m_pScene = scene; };
	void UpdateWorldViewProjectionMatrix(const Matrix& worldMatrix, const Matrix& viewMatrix, const Matrix& projectionMatrix);
	void ToggleClearColor();

protected:
	SDL_Window* m_pWindow{};
	std::shared_ptr<Scene> m_pScene;
	int m_Width{};
	int m_Height{};

	bool m_ClearColor{ false };

	Matrix m_WorldViewProjectionMatrix;
};
