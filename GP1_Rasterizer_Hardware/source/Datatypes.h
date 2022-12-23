#pragma once
#include "Vector3.h"
#include "Vector2.h"
#include "ColorRGB.h"

using namespace dae;

struct Vertex_PosCol
{
	Vector3 position{};
	ColorRGB color{};
	Vector2 uv{};
};

