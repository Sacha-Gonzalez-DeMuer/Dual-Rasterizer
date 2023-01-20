#pragma once
class Material;
class Camera;
class Texture;
struct Vertex;

class Mesh final
{
public:
	Mesh(ID3D11Device* pDevice, ID3D11DeviceContext* pDeviceContext, const std::string& filePath, Material* mat);
	~Mesh();

	Mesh(const Mesh&) = delete;
	Mesh(Mesh&&) = delete;
	Mesh& operator=(const Mesh&) = delete;
	Mesh& operator=(Mesh&&) = delete;

	void Initialize(ID3D11Device* pDevice, std::vector<Vertex> vertices, std::vector<uint32_t> indices);
	void Render(ID3D11Device* pDevice, ID3D11DeviceContext* pDeviceContext, const Camera& pCamera);
private:
	ID3D11Buffer* m_pVertexBuffer;
	ID3D11Buffer* m_pIndexBuffer;
	ID3D11InputLayout* m_pInputLayout;

	Material* m_pMaterial;
	
	uint32_t m_NumIndices{};
	dae::Matrix m_WorldMatrix{};
};

