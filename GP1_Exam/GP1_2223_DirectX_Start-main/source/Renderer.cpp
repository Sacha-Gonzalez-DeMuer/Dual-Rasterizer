
#include "pch.h"

//Project includes
#include "Renderer.h"
#include "Math.h"
#include "Matrix.h"
#include "Texture.h"
#include "Utils.h"

Renderer::Renderer(SDL_Window* pWindow)
	: m_pWindow(pWindow)
{
	//Initialize
	SDL_GetWindowSize(pWindow, &m_Width, &m_Height);
}

Renderer::~Renderer()
{

}

