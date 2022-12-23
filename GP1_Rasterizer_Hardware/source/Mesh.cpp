	#include "pch.h"
#include "Mesh.h"
#include "Effect.h"
#include "Camera.h"
#include "Datatypes.h"
#include "Texture.h"

Mesh::Mesh(ID3D11Device* pDevice, ID3D11DeviceContext* pDeviceContext)
	: m_pEffect{new Effect(pDevice, L"Resources/PosCol3D.fx")}
	, m_NumIndices{0}
	, m_pIndexBuffer{nullptr}
	, m_pInputLayout{nullptr }
{

	//Create some data for our mesh
	std::vector<Vertex_PosCol> vertices
	{
		//pos		//color		//uv
		{{-3,3,-2},		{},		{0,0}		},
		{{0,3,-2},		{},		{.5f,0}		},
		{{3,3,-2},		{},		{1,0}		},
		{{-3,0,-2},		{},		{0,.5f}		},
		{{0,0,-2},		{},		{.5f,.5f}	},
		{{3,0,-2},		{},		{1,.5f}		},
		{{-3,-3,-2},	{},		{0,1}		},
		{{0,-3,-2},		{},		{.5f,1}		},
		{{3,-3,-2},		{},		{1,1}		}
	};

	std::vector<uint32_t> indices
	{ 
		3,0,4,
		0,1,4,
		4,1,5,
		1,2,5,
		6,3,7,
		3,4,7,
		7,4,8,
		4,5,8
	};


	Initialize(pDevice, vertices, indices);
	m_pTexture = Texture::LoadFromFile(pDevice, "Resources/uv_grid_2.png");
	m_pEffect->SetDiffuseMap(m_pTexture);

	m_pEffect->SetSampler(pDevice, pDeviceContext);
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

void Mesh::Initialize(ID3D11Device* pDevice, std::vector<Vertex_PosCol> vertices, std::vector<uint32_t> indices)
{
	m_NumIndices = static_cast<int32_t>(indices.size());

	//Create Vertex Layout
	static constexpr uint32_t numElements{ 3 };
	D3D11_INPUT_ELEMENT_DESC vertexDesc[numElements]{};

	vertexDesc[0].SemanticName = "POSITION";
	vertexDesc[0].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	vertexDesc[0].AlignedByteOffset = 0;
	vertexDesc[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

	vertexDesc[1].SemanticName = "COLOR";
	vertexDesc[1].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	vertexDesc[1].AlignedByteOffset = 12;
	vertexDesc[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

	vertexDesc[2].SemanticName = "TEXCOORD";
	vertexDesc[2].Format = DXGI_FORMAT_R32G32_FLOAT;
	vertexDesc[2].AlignedByteOffset = 20;
	vertexDesc[2].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;


	//Create vertex buffer
	D3D11_BUFFER_DESC bd = {};
	bd.Usage = D3D11_USAGE_IMMUTABLE;
	bd.ByteWidth = sizeof(Vertex_PosCol) * static_cast<uint32_t>(vertices.size());
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
	m_pEffect->GetTechnique()->GetPassByIndex(0)->GetDesc(&passDesc);

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

void Mesh::Render(ID3D11Device* pDevice, ID3D11DeviceContext* pDeviceContext, Camera* pCamera)
{
	//1. Set Primitive Topology
	pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	//2. Set Input Layout
	pDeviceContext->IASetInputLayout(m_pInputLayout);

	//3. Set VertexBuffer
	constexpr UINT stride = sizeof(Vertex_PosCol);
	constexpr UINT offset = 0;
	pDeviceContext->IASetVertexBuffers(0, 1, &m_pVertexBuffer, &stride, &offset);

	//4. Set IndexBuffer
	pDeviceContext->IASetIndexBuffer(m_pIndexBuffer, DXGI_FORMAT_R32_UINT, 0);

	//5. Update worldViewProjectionMatrix
	Matrix worldViewProjectionMatrix{ pCamera->GetViewMatrix() * pCamera->GetProjectionMatrix() };
	m_pEffect->SetMatrix(worldViewProjectionMatrix);


	//6. Draw
	D3DX11_TECHNIQUE_DESC techDesc{};
	m_pEffect->GetTechnique()->GetDesc(&techDesc);
	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		m_pEffect->GetTechnique()->GetPassByIndex(p)->Apply(0, pDeviceContext);
		pDeviceContext->DrawIndexed(m_NumIndices, 0, 0);
	}

}
