#pragma once
#include "Math.h"
#include "vector"

namespace dae
{
	struct Vertex_Out
	{
		Vector4 position{};
		Vector3 worldPos{};
		Vector2 uv{};
		Vector3 normal{};
		Vector3 tangent{};
	};

	struct Vertex
	{
		Vector3 position{};
		Vector3 worldPos{};
		Vector2 uv{};
		Vector3 normal{};
		Vector3 tangent{};
	};

	struct Triangle
	{
		Vertex vertices_in[3]{};
		Vertex_Out vertices[3]{};
		Vector2 edges[3]{};
		float recipTotalArea{};


		void CalculateEdges()
		{
			edges[0] = Vector2(vertices[1].position.x - vertices[0].position.x, vertices[1].position.y - vertices[0].position.y);
			edges[1] = Vector2(vertices[2].position.x - vertices[1].position.x, vertices[2].position.y - vertices[1].position.y);
			edges[2] = Vector2(vertices[0].position.x - vertices[2].position.x, vertices[0].position.y - vertices[2].position.y);
		}

		float GetArea() const
		{
			return Vector2::Cross(edges[0], edges[1]);
		}


		Vector2 GetVector2Pos(int idx) const
		{
			return { vertices[idx].position.x, vertices[idx].position.y };
		};

		constexpr float GetInterpolatedZ(const float(&weights)[3]) const
		{
			return
			{
				1 / (
					((1 / vertices[0].position.z) * weights[1]) +
					((1 / vertices[1].position.z) * weights[2]) +
					((1 / vertices[2].position.z) * weights[0]))
			};
		}

		constexpr float GetInterpolatedW(const float(&weights)[3]) const
		{
			return
			{
				1 / (
				((1 / vertices[0].position.w) * weights[1]) +
				((1 / vertices[1].position.w) * weights[2]) +
				((1 / vertices[2].position.w) * weights[0]))
			};
		};
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

	enum class CullMode
	{
		Backface,
		Frontface,
		None,
		Size = 3
	};

	enum class SamplerState
	{
		Point,
		Linear,
		Anisotropic,
		Size = 3
	};
}