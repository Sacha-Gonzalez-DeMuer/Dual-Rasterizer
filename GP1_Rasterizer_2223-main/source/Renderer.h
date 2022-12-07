#pragma once

#include <cstdint>
#include <vector>

#include "Camera.h"
#include "DataTypes.h"

struct SDL_Window;
struct SDL_Surface;

namespace dae
{
	class Texture;
	struct Mesh;
	struct Vertex;
	class Timer;
	class Scene;

	class Renderer final
	{
	public:
		Renderer(SDL_Window* pWindow);
		~Renderer();

		Renderer(const Renderer&) = delete;
		Renderer(Renderer&&) noexcept = delete;
		Renderer& operator=(const Renderer&) = delete;
		Renderer& operator=(Renderer&&) noexcept = delete;

		void Update(Timer* pTimer);
		void Render();

		bool SaveBufferToImage() const;
		void CycleRenderMode();

	private:
		void Render_W1_Part1();
		void Render_W1_Part2();
		void Render_W1_Part3();
		void Render_W1_Part4();
		void Render_W1_Part5();

		void Render_W2_Part1(); //triangle list & strip
		void Render_W2_Part2(); //uv

		void Render_W3_Part1();
		void PixelLoop(const Triangle& t, const BoundingBox& bb);

		SDL_Window* m_pWindow{};

		SDL_Surface* m_pFrontBuffer{ nullptr };
		SDL_Surface* m_pBackBuffer{ nullptr };
		uint32_t* m_pBackBufferPixels{};

		float* m_pDepthBufferPixels{};

		std::vector<Vertex> m_vertices_screenSpace;
		Camera m_Camera{};
		Matrix m_WorldMatrix{};
		Matrix m_ViewMatrix{};
		Matrix m_ProjectionMatrix{};

		Matrix m_WorldViewProjectionMatrix{ };
	
		RenderMode m_CurrentRenderMode{ RenderMode::DepthBuffer };

		Texture* m_pTexture{ nullptr };

		int m_Width{};
		int m_Height{};

		//Function that transforms the vertices from the mesh from World space to Screen space
		void VertexTransformationFunction(const std::vector<Vertex>& vertices_in, std::vector<Vertex>& vertices_out) const; //W1 Version
		void VertexTransformationFunction(const std::vector<Vertex>& vertices_in, std::vector<Vertex_Out>& vertices_out) const; //W2 Version
		
		void VertexTransformationFunction(Mesh& mesh); //W3 Version - worldviewprojection

		bool IsVertexInFrustum(const Vertex_Out& v);
		bool IsVertexInFrustum(const Vector4 v);
		bool IsTriangleInFrustum(const Triangle& t);

		bool IsPointInTri(Vector2 P, const Vector2 vertexPositions[], float (&weights)[3]) const;
		bool IsPointInTri(Vector2 P, const Triangle& t, float(&weights)[3]) const;

		BoundingBox GenerateBoundingBox(const Vector2 vertices[]) const;
		BoundingBox GenerateBoundingBox(const Triangle t) const;

		float Remap(float value, float rangeMin, float rangeMax);

		void UpdateWorldViewProjectionMatrix(const Matrix& worldMatrix, const Matrix& viewMatrix, const Matrix& projectionMatrix);
		void ParseMesh(Mesh mesh, std::vector<Vertex>& vertices_out);
	};
}
