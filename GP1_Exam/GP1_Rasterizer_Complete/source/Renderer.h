#pragma once

#include <cstdint>
#include <vector>

struct SDL_Window;

class Scene;
class Timer;

class Renderer
{
public:
	Renderer() = delete;
	Renderer(SDL_Window* pWindow, std::shared_ptr<Scene> sceneToLoad);
	virtual ~Renderer();

	Renderer(const Renderer&) = delete;
	Renderer(Renderer&&) noexcept = delete;
	Renderer& operator=(const Renderer&) = delete;
	Renderer& operator=(Renderer&&) noexcept = delete;

	virtual void Update(Timer* pTimer) = 0;
	virtual void Render() = 0;

	//bool SaveBufferToImage() const;
protected:
	SDL_Window* m_pWindow{};
	std::shared_ptr<Scene> m_pScene;
	int m_Width{};
	int m_Height{};
};
