	#include "pch.h"
#include "Mesh.h"
#include "MeshMaterial.h"
#include "Camera.h"
#include "Datatypes.h"
#include "Texture.h"
#include "Utils.h"

Mesh::Mesh(ID3D11Device* pDevice, ID3D11DeviceContext* pDeviceContext, const std::string& filePath, Material* mat)
	: m_pMaterial{mat}
	, m_NumIndices{0}
	, m_pIndexBuffer{nullptr}
	, m_pInputLayout{nullptr }
{
	std::vector<Vertex> vertices{};
	std::vector<uint32_t> indices{};
	dae::Utils::ParseOBJ(filePath, vertices, indices, true);
	
	Initialize(pDevice, vertices, indices);
}

Mesh::~Mesh()
{
	m_pVertexBuffer->Release();
	m_pIndexBuffer->Release();
	m_pInputLayout->Release();

	m_pVertexBuffer = nullptr;
	m_pIndexBuffer = nullptr;
	m_pInputLayout = nullptr;
}

void Mesh::Initialize(ID3D11Device* pDevice, std::vector<Vertex> vertices, std::vector<uint32_t> indices)
{
	m_NumIndices = static_cast<int32_t>(indices.size());

	//Create Vertex Layout
	static constexpr uint32_t numElements{ 5 };
	D3D11_INPUT_ELEMENT_DESC vertexDesc[numElements]{};

	vertexDesc[0].SemanticName = "POSITION";
	vertexDesc[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	vertexDesc[0].AlignedByteOffset = 0;
	vertexDesc[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

	vertexDesc[1].SemanticName = "WORLD";
	vertexDesc[1].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	vertexDesc[1].AlignedByteOffset = 12;
	vertexDesc[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

	vertexDesc[2].SemanticName = "TEXCOORD";
	vertexDesc[2].Format = DXGI_FORMAT_R32G32_FLOAT;
	vertexDesc[2].AlignedByteOffset = 24;
	vertexDesc[2].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

	vertexDesc[3].SemanticName = "NORMAL";
	vertexDesc[3].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	vertexDesc[3].AlignedByteOffset = 32;
	vertexDesc[3].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

	vertexDesc[4].SemanticName = "TANGENT";
	vertexDesc[4].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	vertexDesc[4].AlignedByteOffset = 44;
	vertexDesc[4].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

	//Create vertex buffer
	D3D11_BUFFER_DESC bd = {};
	bd.Usage = D3D11_USAGE_IMMUTABLE;
	bd.ByteWidth = sizeof(Vertex) * static_cast<uint32_t>(vertices.size());
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;
	bd.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA initData{};
	initData.pSysMem = vertices.data();

	HRESULT result = pDevice->CreateBuffer(&bd, &initData, &m_pVertexBuffer);
	if (FAILED(result))
		return;

	//Create Input Layout
	D3DX11_PASS_DESC passDesc{};
	m_pMaterial->GetTechnique()->GetPassByIndex(0)->GetDesc(&passDesc);

	result = pDevice->CreateInputLayout(
		vertexDesc,
		numElements,
		passDesc.pIAInputSignature,
		passDesc.IAInputSignatureSize,
		&m_pInputLayout);

	if (FAILED(result))
		return;

	//Create index buffer
	m_NumIndices = static_cast<uint32_t>(indices.size());
	bd.Usage = D3D11_USAGE_IMMUTABLE;
	bd.ByteWidth = sizeof(uint32_t) * m_NumIndices;
	bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bd.CPUAccessFlags = 0;
	bd.MiscFlags = 0;
	initData.pSysMem = indices.data();
	result = pDevice->CreateBuffer(&bd, &initData, &m_pIndexBuffer);

	if (FAILED(result))
		return;
}

void Mesh::Render(ID3D11Device* pDevice, ID3D11DeviceContext* pDeviceContext, const Camera& camera)
{
	//1. Set Primitive Topology
	pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	//2. Set Input Layout
	pDeviceContext->IASetInputLayout(m_pInputLayout);

	//3. Set VertexBuffer
	constexpr UINT stride = sizeof(Vertex);
	constexpr UINT offset = 0;
	pDeviceContext->IASetVertexBuffers(0, 1, &m_pVertexBuffer, &stride, &offset);

	//4. Set IndexBuffer
	pDeviceContext->IASetIndexBuffer(m_pIndexBuffer, DXGI_FORMAT_R32_UINT, 0);

	//5. Update effect matrices
	Matrix worldViewProjectionMatrix{ camera.GetViewMatrix() * camera.GetProjectionMatrix() };
	m_pMaterial->UpdateEffect(m_WorldMatrix, camera.GetInvViewMatrix(), worldViewProjectionMatrix);

	//7. Draw
	D3DX11_TECHNIQUE_DESC techDesc{};
	m_pMaterial->GetTechnique()->GetDesc(&techDesc);
	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		m_pMaterial->GetTechnique()->GetPassByIndex(p)->Apply(0, pDeviceContext);
		pDeviceContext->DrawIndexed(m_NumIndices, 0, 0);
	}
}
