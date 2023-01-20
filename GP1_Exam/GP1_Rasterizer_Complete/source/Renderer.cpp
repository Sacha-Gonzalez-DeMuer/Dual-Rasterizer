//External includes
#include "SDL.h"
#include "SDL_surface.h"

//Project includes
#include "Renderer.h"
#include "Math.h"
#include "Matrix.h"
#include "Texture.h"
#include "Utils.h"

Renderer::Renderer(SDL_Window* pWindow, std::shared_ptr<Scene> sceneToLoad)
	: m_pWindow(pWindow)
{
	//Initialize
	SDL_GetWindowSize(pWindow, &m_Width, &m_Height);

}

Renderer::~Renderer()
{
	delete m_pWindow;
	m_pWindow = nullptr;
}

