#include "pch.h"

#if defined(_DEBUG)
#include "vld.h"
#endif

#undef main
#include "Timer.h"
#include "Renderer.h"
#include "CombinedRenderer.h"
#include "MeshManager.h"
#include "FilePaths.h"

void ShutDown(SDL_Window* pWindow)
{
	delete MeshManager::Get();

	SDL_DestroyWindow(pWindow);
	SDL_Quit();
}

int main(int argc, char* args[])
{
	//Unreferenced parameters
	(void)argc;
	(void)args;

	//Create window + surfaces
	SDL_Init(SDL_INIT_VIDEO);

	const uint32_t width = 640;
	const uint32_t height = 480;

	SDL_Window* pWindow = SDL_CreateWindow(
		"DirectX - ***Insert Name/Class***",
		SDL_WINDOWPOS_UNDEFINED,
		SDL_WINDOWPOS_UNDEFINED,
		width, height, 0);

	if (!pWindow)
		return 1;

	//Initialize "framework"
	const auto pTimer = new dae::Timer();
	const auto pRenderer = new CombinedRenderer(pWindow);

	//Start loop
	pTimer->Start();
	float printTimer = 0.f;
	bool isLooping = true;
	while (isLooping)
	{
		//--------- Get input events ---------
		SDL_Event e;
		while (SDL_PollEvent(&e))
		{
			switch (e.type)
			{
			case SDL_QUIT:
				isLooping = false;
				break;
			case SDL_KEYUP:
				//Test for a key
				if (e.key.keysym.scancode == SDL_SCANCODE_F1)
					pRenderer->ToggleGPU();

				if (e.key.keysym.scancode == SDL_SCANCODE_F2)
					pRenderer->ToggleRotation();

				if(e.key.keysym.scancode == SDL_SCANCODE_F3)
					MeshManager::Get()->GetMesh(FILE_OBJ_FIREFX)->ToggleRender();

				if (e.key.keysym.scancode == SDL_SCANCODE_F9)
					pRenderer->CycleCullMode();

				if (e.key.keysym.scancode == SDL_SCANCODE_F10)
					pRenderer->ToggleClearColor();

				if (e.key.keysym.scancode == SDL_SCANCODE_F11)
					pTimer->TogglePrintFPW();

				break;
			default: ;
			}
		}

		//--------- Update ---------
		pRenderer->Update(pTimer);

		//--------- Render ---------
		pRenderer->Render();

		//--------- Timer ---------
		pTimer->Update();
		printTimer += pTimer->GetElapsed();
		if (pTimer->PrintFPW() && printTimer >= 1.f)
		{
			printTimer = 0.f;
			std::cout << "dFPS: " << pTimer->GetdFPS() << std::endl;
		}
	}
	pTimer->Stop();

	//Shutdown "framework"
	delete pRenderer;
	delete pTimer;

	ShutDown(pWindow);
	return 0;
}