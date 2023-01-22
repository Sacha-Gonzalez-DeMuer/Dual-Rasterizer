#pragma once
class Camera;
class Material;
using namespace dae;

class Mesh
{
public:
	Mesh(const std::string& filePath);
	~Mesh();

	void SInitialize();
	void DXInitialize(ID3D11Device* pDevice);

	void Update(const dae::Timer* pTimer);

	void DXRender(ID3D11Device* pDevice, ID3D11DeviceContext* pDeviceContext, const Camera& camera);
	ColorRGB PixelShading(const Vertex_Out& v, const Camera& camera) const;

	void SetMaterial(const std::shared_ptr<Material>& mat);
	std::vector<dae::Vertex> GetVertices() const { return m_Vertices; };
	std::vector<dae::Vertex_Out> GetTransformedVertices() const { return m_TransformedVertices; };
	const dae::Matrix& GetWorldMatrix() const { return m_WorldMatrix; };

	std::vector<Vertex_Out>& GetVerticesOut() { return m_TransformedVertices; };
	std::vector<Triangle>& GetTriangles() { return m_Triangles; };
	std::shared_ptr<Material> GetMaterial() const { return m_pMaterial; };

	bool SInitialized() { return m_SInitialized; };
	bool DXInitialized() { return m_DXInitialized; };


	// SRenderer
	void TriangleTransformationFunction(const Matrix& worldViewProjectionMatrix);
	void VertexTransformationFunction(const Matrix& worldViewProjectionMatrix);


	// Control
	void SetRotation(bool rotate);
	void SetCullMode(CullMode mode, ID3D11Device* pDevice, ID3D11DeviceContext* pDeviceContext);
	void SetSampleState(SamplerState state, ID3D11Device* pDevice, ID3D11DeviceContext* pDeviceContext);
	void ToggleRender();

private:
	bool m_DXInitialized;
	bool m_SInitialized;
	bool m_RenderEnabled;

	bool m_RotationEnabled;
	std::vector<dae::Vertex> m_Vertices;
	std::vector<uint32_t> m_Indices;

	std::shared_ptr<Material> m_pMaterial;
	dae::Matrix m_WorldMatrix;
	Vector3 m_YawPitchRoll;

	// DX11 members
	ID3D11Buffer* m_pVertexBuffer;
	ID3D11Buffer* m_pIndexBuffer;
	ID3D11InputLayout* m_pInputLayout;
	uint32_t m_NumIndices;

	// Software members
	std::vector<Vertex_Out> m_TransformedVertices;
	std::vector<Triangle> m_Triangles;
};

