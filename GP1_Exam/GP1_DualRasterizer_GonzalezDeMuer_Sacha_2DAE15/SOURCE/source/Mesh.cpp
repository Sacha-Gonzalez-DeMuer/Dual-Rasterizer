#include "pch.h"
#include "Utils.h"
#include "Material.h"
#include "Mesh.h"
#include "Camera.h"

Mesh::Mesh(const std::string& filePath)
	: m_pVertexBuffer{nullptr}
	, m_pIndexBuffer{nullptr}
	, m_pInputLayout{nullptr}
	, m_pMaterial{nullptr}
	, m_WorldMatrix{}
	, m_YawPitchRoll{}
	, m_RotationEnabled{true}
	, m_Vertices{}
	, m_Indices{}
	, m_NumIndices{}
	, m_RenderEnabled{true}
	, m_DXInitialized{false}
	, m_SInitialized{false}
{
	dae::Utils::ParseOBJ(filePath, m_Vertices, m_Indices, false);
}

Mesh::~Mesh()
{
	m_pVertexBuffer->Release();
	m_pVertexBuffer = nullptr;

	m_pIndexBuffer->Release();
	m_pIndexBuffer = nullptr;

	m_pInputLayout->Release();
	m_pInputLayout = nullptr;
}

void Mesh::SInitialize()
{
	m_TransformedVertices.resize(m_Vertices.size());

	m_Triangles.resize(m_Vertices.size() / 3);
	for (size_t i = 0; i < m_Vertices.size(); i += 3)
	{
		for (size_t j = 0; j < 3; ++j)
		{
			m_Triangles[i / 3].vertices_in[j] = m_Vertices[i + j];
		}
	}

	for (auto& t : m_Triangles)
	{
		t.CalculateEdges();
		t.recipTotalArea = 1 / t.GetArea();
	}

	m_SInitialized = true;
}

void Mesh::DXInitialize(ID3D11Device* pDevice)
{
	m_NumIndices = static_cast<int32_t>(m_Indices.size());

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
	bd.ByteWidth = sizeof(Vertex) * static_cast<uint32_t>(m_Vertices.size());
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;
	bd.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA initData{};
	initData.pSysMem = m_Vertices.data();

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
	m_NumIndices = static_cast<uint32_t>(m_Indices.size());
	bd.Usage = D3D11_USAGE_IMMUTABLE;
	bd.ByteWidth = sizeof(uint32_t) * m_NumIndices;
	bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bd.CPUAccessFlags = 0;
	bd.MiscFlags = 0;
	initData.pSysMem = m_Indices.data();
	result = pDevice->CreateBuffer(&bd, &initData, &m_pIndexBuffer);

	if (FAILED(result))
		return;

	m_DXInitialized = true;
}

void Mesh::Update(const dae::Timer* pTimer)
{
	if (m_RotationEnabled)
	{
		m_YawPitchRoll.x += pTimer->GetElapsed();
		m_WorldMatrix = Matrix::CreateRotationY(m_YawPitchRoll.x);
	}
}

void Mesh::DXRender(ID3D11Device* pDevice, ID3D11DeviceContext* pDeviceContext, const Camera& camera)
{
	if (!m_RenderEnabled || !m_DXInitialized) return;

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
	Matrix worldViewProjectionMatrix{ m_WorldMatrix * camera.GetViewMatrix() * camera.GetProjectionMatrix() };
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

void Mesh::SetMaterial(const std::shared_ptr<Material>& mat)
{
	m_pMaterial = mat;
}

ColorRGB Mesh::PixelShading(const Vertex_Out& v, const Camera& camera) const
{
	return m_pMaterial->PixelShading(v, *this, camera);
}

void Mesh::TriangleTransformationFunction(const Matrix& worldViewProjectionMatrix)
{
	for (auto& triangle : m_Triangles)
	{
		for (int i = 0; i < 3; ++i)
		{
			triangle.vertices[i].position = { triangle.vertices_in[i].position.x, triangle.vertices_in[i].position.y, triangle.vertices_in[i].position.z, 1 };

			//transform normals & tangents from model space to world space
			Matrix worldMatrix3x3{ m_WorldMatrix.GetAxisX(), m_WorldMatrix.GetAxisY(), m_WorldMatrix.GetAxisZ(), Vector3::Zero };
			triangle.vertices[i].normal = worldMatrix3x3.TransformVector(triangle.vertices_in[i].normal);
			triangle.vertices[i].tangent = worldMatrix3x3.TransformVector(triangle.vertices_in[i].tangent);

			//position to clip space
			triangle.vertices[i].position = worldViewProjectionMatrix.TransformPoint(triangle.vertices[i].position);

			const float recipW{ 1 / triangle.vertices[i].position.w };

			//apply perspective divide => clip space -> NDC space 
			triangle.vertices[i].position.x *= recipW;
			triangle.vertices[i].position.y *= recipW;
			triangle.vertices[i].position.z *= recipW;

			// Copy the UV coordinates from the input vertex
			triangle.vertices[i].uv = triangle.vertices_in[i].uv;
		}
	}
}

void Mesh::VertexTransformationFunction(const Matrix& worldViewProjectionMatrix)
{	
	//auto& verticesOut{ mesh->GetVerticesOut() };

	// Transform the vertices from world space to clip space
	for (size_t i = 0; i < m_Vertices.size(); ++i)
	{
		m_TransformedVertices[i].position = { m_Vertices[i].position.x, m_Vertices[i].position.y, m_Vertices[i].position.z, 1 };

		//transform normals & tangents from model space to world space
		Matrix worldMatrix3x3{ m_WorldMatrix.GetAxisX(),  m_WorldMatrix.GetAxisY(), m_WorldMatrix.GetAxisZ(), Vector3::Zero};
		m_TransformedVertices[i].normal = worldMatrix3x3.TransformVector(m_Vertices[i].normal);
		m_TransformedVertices[i].tangent = worldMatrix3x3.TransformVector(m_Vertices[i].tangent);

		//position to clip space
		m_TransformedVertices[i].position = worldViewProjectionMatrix.TransformPoint(m_TransformedVertices[i].position);

		const float recipW{ 1 / m_TransformedVertices[i].position.w };

		//apply perspective divide => clip space -> NDC space
		m_TransformedVertices[i].position.x *= recipW;
		m_TransformedVertices[i].position.y *= recipW;
		m_TransformedVertices[i].position.z *= recipW;

		// Copy the UV coordinates from the input vertex
		m_TransformedVertices[i].uv = m_Vertices[i].uv;
	}
}

void Mesh::SetRotation(bool rotate)
{
	m_RotationEnabled = rotate;
}

void Mesh::SetCullMode(CullMode mode, ID3D11Device* pDevice, ID3D11DeviceContext* pDeviceContext)
{
	m_pMaterial->SetCullMode(mode, pDevice, pDeviceContext);
}

void Mesh::SetSampleState(SamplerState state, ID3D11Device* pDevice, ID3D11DeviceContext* pDeviceContext)
{
	m_pMaterial->SetSampler(state, pDevice, pDeviceContext);
}

void Mesh::ToggleRender()
{
	m_RenderEnabled = !m_RenderEnabled;

	std::cout << "TOGGLED MESH RENDER: ";
	m_RenderEnabled ? std::cout << "ON\n" : std::cout << "OFF\n";
}
