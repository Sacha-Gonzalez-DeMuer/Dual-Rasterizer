#include "pch.h"
#include "Material.h"
#include "Texture.h"

Material::Material(ID3D11Device* pDevice, const std::wstring& assetFile)
    : m_pEffect{ LoadEffect(pDevice, assetFile) }
    , m_UseNormalMap{ true }
{
    // Set Technique
    m_pTechnique = m_pEffect->GetTechniqueByName("DefaultTechnique");
    if (!m_pTechnique->IsValid())
        std::wcout << L"Technique not valid\n";

    // Get FX Sampler
    m_pSampleStateVariable = m_pEffect->GetVariableByName("gSamplePoint")->AsSampler();

    // Get FX Rasterizer
    m_pRasterizerVariable = m_pEffect->GetVariableByName("gRasterizerState")->AsRasterizer();

    SetMatrices();
}

Material::~Material()
{
    m_pTechnique->Release();
    m_pTechnique = nullptr;

    m_pEffect->Release();
    m_pEffect = nullptr;

}

void Material::UpdateEffect(const Matrix& worldMatrix, const Matrix& invViewMatrix, const Matrix& worldViewProjection)
{
    SetWorldViewProjectionMatrix(worldViewProjection);
    SetWorldMatrix(worldMatrix);
    SetInvViewMatrix(invViewMatrix);
}

void Material::SetMatrices()
{
    // World View Projection
    m_pWorldViewProjMax = m_pEffect->GetVariableByName("gWorldViewProj")->AsMatrix();
    if (!m_pWorldViewProjMax->IsValid())
        std::wcout << L"m_pMatWorldViewProjVariable not valid!\n";

    // Inverse view
    m_pInvViewMatrix = m_pEffect->GetVariableByName("gInvViewMatrix")->AsMatrix();
    if (!m_pInvViewMatrix)
        std::wcout << L"m_pInvViewMatrix not valid!\n";

    // World
    m_pWorldMatrix = m_pEffect->GetVariableByName("gWorldMatrix")->AsMatrix(); \
        if (!m_pWorldMatrix)
            std::wcout << L"m_pInvViewMatrix not valid!\n";
}


void Material::SetWorldViewProjectionMatrix(const Matrix& worldViewProjectionMatrix)
{
    m_pWorldViewProjMax->SetMatrix(reinterpret_cast<const float*>(&worldViewProjectionMatrix));
}

void Material::SetWorldMatrix(const Matrix& worldMatrix)
{
    m_pWorldMatrix->SetMatrix(reinterpret_cast<const float*>(&worldMatrix));
}

void Material::SetInvViewMatrix(const Matrix& invViewMatrix)
{
    m_pInvViewMatrix->SetMatrix(reinterpret_cast<const float*>(&invViewMatrix));
}

ID3DX11Effect* Material::LoadEffect(ID3D11Device* pDevice, const std::wstring& assetFile)
{
    HRESULT result;
    ID3D10Blob* pErrorBlob{ nullptr };
    ID3DX11Effect* pEffect;

    DWORD shaderFlags = 0;

#if defined(DEBUG)|| defined(_DEBUG)
    shaderFlags |= D3DCOMPILE_DEBUG;
    shaderFlags |= D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

    result = D3DX11CompileEffectFromFile(assetFile.c_str(),
        nullptr,
        nullptr,
        shaderFlags,
        0,
        pDevice,
        &pEffect,
        &pErrorBlob);

    if (FAILED(result))
    {
        if (pErrorBlob != nullptr)
        {
            const char* pErrors = static_cast<char*>(pErrorBlob->GetBufferPointer());

            std::wstringstream ss;
            for (unsigned int i = 0; i < pErrorBlob->GetBufferSize(); i++)
                ss << pErrors[i];

            OutputDebugStringW(ss.str().c_str());
            pErrorBlob->Release();
            pErrorBlob = nullptr;

            std::wcout << ss.str() << "\n";
        }
        else
        {
            std::wstringstream ss;
            ss << "EffectLoader: Failed to CreateEffectFromFile!\nPath: " << assetFile;
            std::wcout << ss.str() << std::endl;
            return nullptr;
        }
    }

    return pEffect;
}


void Material::SetSampler(SamplerState state, ID3D11Device* pDevice, ID3D11DeviceContext* pDeviceContext)
{
    //https://learn.microsoft.com/en-us/windows/win32/api/d3d11/ns-d3d11-d3d11_sampler_desc
    D3D11_SAMPLER_DESC desc{};

    switch (state)
    {
    case SamplerState::Point:
        desc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
        break;

    case SamplerState::Linear:
        desc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
        break;

    case SamplerState::Anisotropic:
        desc.Filter = D3D11_FILTER_ANISOTROPIC;
        break;
    }
 
    desc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    desc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    desc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    desc.MipLODBias = 0;
    desc.MaxAnisotropy = 0;
    desc.ComparisonFunc = D3D11_COMPARISON_NEVER;
    desc.MinLOD = 0;
    desc.MaxLOD = D3D11_FLOAT32_MAX;

    ID3D11SamplerState* pSamplerState{ nullptr };
    HRESULT result = pDevice->CreateSamplerState(&desc, &pSamplerState);
    if (FAILED(result))
    {
        std::cout << "Failed to create sampler state\n";
        return;
    }

    pDeviceContext->PSSetSamplers(0, 1, &pSamplerState);
    m_pSampleStateVariable->SetSampler(0, pSamplerState);
}

std::shared_ptr<Texture> Material::SetTexture(ID3D11Device* pDevice, ID3DX11EffectShaderResourceVariable* srv, const std::string& assetFile)
{
    auto pTexture{ Texture::LoadFromFile(pDevice, assetFile) };
    if(srv)
     srv->SetResource(pTexture->GetSRV());
    return pTexture;
}

void Material::SetCullMode(CullMode mode, ID3D11Device* pDevice, ID3D11DeviceContext* pDeviceContext)
{
    D3D11_RASTERIZER_DESC rasterDesc;

    rasterDesc.FillMode = D3D11_FILL_SOLID;
    switch (mode)
    {
    case CullMode::Backface:
        rasterDesc.CullMode = D3D11_CULL_BACK;
        break;

    case CullMode::Frontface:
        rasterDesc.CullMode = D3D11_CULL_FRONT;
        break;

    case CullMode::None:
        rasterDesc.CullMode = D3D11_CULL_NONE;
        break;
    }

    rasterDesc.FrontCounterClockwise = true;
    rasterDesc.DepthBias = 0;
    rasterDesc.DepthBiasClamp = 0.0f;
    rasterDesc.SlopeScaledDepthBias = 0.0f;
    rasterDesc.DepthClipEnable = true;
    rasterDesc.ScissorEnable = false;
    rasterDesc.MultisampleEnable = false;
    rasterDesc.AntialiasedLineEnable = false;

    ID3D11RasterizerState* rasterState;
    HRESULT hr = pDevice->CreateRasterizerState(&rasterDesc, &rasterState);
    if (FAILED(hr))
    {
        std::cout << "Failed to set cull mode\n";
        return;
    }

    pDeviceContext->RSSetState(rasterState);
    m_pRasterizerVariable->SetRasterizerState(0, rasterState);
}

ColorRGB Material::Sample(const Vector2& uv, const Texture& texture)
{
    return ColorRGB();
}

void Material::SetShadingMode(ShadingMode mode)
{
    m_ShadingMode = mode;
}

void Material::UseNormalMap(bool use)
{
    m_UseNormalMap = use;
}


