#pragma once

#include <iostream>
#include <vector>
#include <algorithm>
#include <sstream>
#include <memory>
#define NOMINMAX  //for directx

// SDL Headers
#include "SDL.h"
#include "SDL_syswm.h"
#include "SDL_surface.h"
#include "SDL_image.h"

// DirectX Headers
#include <dxgi.h> //dx11 api
#include <d3d11.h> //dx11 shader compiler functionality
#include <d3dcompiler.h> //dxgi api
#include <d3dx11effect.h> //external effect framework

// Framework Headers
#include "Timer.h"
#include "Math.h"