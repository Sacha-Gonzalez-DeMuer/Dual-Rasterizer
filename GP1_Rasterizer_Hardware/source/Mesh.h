#pragma once
class Effect;
class Camera;
class Texture;
struct Vertex_PosCol;

class Mesh final
{
public:
	Mesh(ID3D11Device* pDevice, ID3D11DeviceContext* pDeviceContext);
	~Mesh();

	Mesh(const Mesh&) = delete;
	Mesh(Mesh&&) = delete;
	Mesh& operator=(const Mesh&) = delete;
	Mesh& operator=(Mesh&&) = delete;

	void Initialize(ID3D11Device* pDevice, std::vector<Vertex_PosCol> vertices, std::vector<uint32_t> indices);
	void Render(ID3D11Device* pDevice, ID3D11DeviceContext* pDeviceContext, Camera* pCamera);
private:
	//std::vector<dae::Vector3> m_Vertices{};
	//std::vector<int> m_Indices{};

	ID3D11Buffer* m_pVertexBuffer;
	ID3D11Buffer* m_pIndexBuffer;
	ID3D11InputLayout* m_pInputLayout;

	std::unique_ptr<Effect> m_pEffect;
	Texture* m_pTexture{};
	uint32_t m_NumIndices{};
};

