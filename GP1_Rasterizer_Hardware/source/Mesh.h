#pragma once

class Effect;

struct Vertex_PosCol
{
	dae::Vector3 position{};
	dae::ColorRGB color{};
};


class Mesh
{
public:
	Mesh(ID3D11Device* pDevice);

	void Initialize(ID3D11Device* pDevice, std::vector<Vertex_PosCol> vertices, std::vector<uint32_t> indices);
	void Render(ID3D11DeviceContext* pDeviceContext);
	
private:
	//std::vector<dae::Vector3> m_Vertices{};
	//std::vector<int> m_Indices{};

	ID3D11Buffer* m_pVertexBuffer;
	ID3D11Buffer* m_pIndexBuffer;
	ID3D11InputLayout* m_pInputLayout;

	Effect* m_pEffect;

	uint32_t m_NumIndices{};
};

