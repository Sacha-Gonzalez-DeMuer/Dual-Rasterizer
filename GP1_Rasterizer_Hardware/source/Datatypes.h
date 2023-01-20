#pragma once
#include "Vector3.h"
#include "Vector2.h"
#include "ColorRGB.h"

struct Vertex
{
	dae::Vector3 position{};
	dae::Vector3 worldPos{};
	dae::Vector2 uv{};
	dae::Vector3 normal{};
	dae::Vector3 tangent{};
};

