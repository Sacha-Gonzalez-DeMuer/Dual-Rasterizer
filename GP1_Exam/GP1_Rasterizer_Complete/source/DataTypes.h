#pragma once
#include "Math.h"
#include "vector"

namespace dae
{
	struct Vertex
	{
		Vector3 position{};
		Vector3 worldPos{};
		Vector2 uv{};
		Vector3 normal{};
		Vector3 tangent{};
	};

	struct Vertex_Out
	{
		Vector4 position{};
		Vector3 worldPos{};
		Vector2 uv{};
		Vector3 normal{};
		Vector3 tangent{};
	};

	struct BoundingBox
	{
		int minX;
		int minY;
		int maxX;
		int maxY;
	};

	enum class PrimitiveTopology
	{
		TriangeList,
		TriangleStrip
	};

	enum class RenderMode
	{
		FinalColor,
		DepthBuffer,
		Size = 2
	};

	enum class ShadingMode
	{
		ObservedArea,
		Diffuse,
		Specular,
		Combined,
		Size = 4
	};

	struct Mesh
	{
		std::vector<Vertex> vertices{};
		std::vector<uint32_t> indices{};
		PrimitiveTopology primitiveTopology{ PrimitiveTopology::TriangleStrip };

		std::vector<Vertex_Out> vertices_out{};
		Matrix worldMatrix{};
	};
}