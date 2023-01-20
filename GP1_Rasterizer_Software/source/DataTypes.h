#pragma once
#include "Math.h"
#include "vector"
#include <array>

namespace dae
{
	struct Vertex
	{
		Vector3 position{};
		ColorRGB color{colors::White};
		Vector2 uv{}; 
		Vector3 normal{}; 
		Vector3 tangent{}; 
		//Vector3 viewDirection{}; //W4
	};

	struct Vertex_Out
	{
		Vector4 position{};
		ColorRGB color{ colors::White };
		Vector2 uv{};
		Vector3 normal{};
		Vector3 tangent{};
		//Vector3 viewDirection{};
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
		Vector3 yawPitchRoll{};
	};



	struct Triangle
	{
		Vertex_Out vertices[3]{};
		Vector2 edges[3]{};
		float recipTotalArea{};


		Vector2 GetVector2Pos(int idx) const 
		{ 
			return { vertices[idx].position.x, vertices[idx].position.y };
		};

		float GetInterpolatedZ(const float(&weights)[3]) const
		{
			return
			{
				1 / (
					((1 / vertices[0].position.z) * weights[1]) +
					((1 / vertices[1].position.z) * weights[2]) +
					((1 / vertices[2].position.z) * weights[0]))
			};


		}

		float GetInterpolatedW(const float (&weights)[3]) const
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


}