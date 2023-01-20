#pragma once
class Camera;
class Material;

class Mesh
{
public:
	Mesh(const std::string& filePath);

	void DXInitialize(ID3D11Device* pDevice);

	void SRender();
	void DXRender(ID3D11Device* pDevice, ID3D11DeviceContext* pDeviceContext, const Camera& camera);

	void SetMaterial(const std::shared_ptr<Material>& mat);

private:
	std::vector<dae::Vertex> m_Vertices;
	std::vector<uint32_t> m_Indices;

	std::shared_ptr<Material> m_pMaterial;
	dae::Matrix m_WorldMatrix{};

	// DX11 members
	ID3D11Buffer* m_pVertexBuffer;
	ID3D11Buffer* m_pIndexBuffer;
	ID3D11InputLayout* m_pInputLayout;

	uint32_t m_NumIndices;
};

