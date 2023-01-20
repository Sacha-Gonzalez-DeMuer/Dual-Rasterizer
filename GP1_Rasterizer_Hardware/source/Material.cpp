#include "pch.h"
#include "Material.h"
#include "Texture.h"

Material::Material(ID3D11Device* pDevice, const std::wstring& assetFile)
    : m_pEffect{ LoadEffect(pDevice, assetFile) }
{
    // Set Technique
    m_pTechnique = m_pEffect->GetTechniqueByName("DefaultTechnique");
    if (!m_pTechnique->IsValid())
        std::wcout << L"Technique not valid\n";

    // Set Sampler
    m_pSampleStateVariable = m_pEffect->GetVariableByName("gSamplePoint")->AsSampler();

    SetMatrices();
}

Material::~Material()
{
    m_pTechnique->Release();
    m_pTechnique = nullptr;

    m_pEffect->Release();
    m_pEffect = nullptr;

    m_pWorldViewProjMax->Release();
    m_pWorldViewProjMax = nullptr;

    m_pWorldMatrix->Release();
    m_pWorldMatrix = nullptr;

    m_pInvViewMatrix->Release();
    m_pInvViewMatrix = nullptr;

    m_pSamplerState->Release();
    m_pSamplerState = nullptr;

    m_pSampleStateVariable->Release();
    m_pSampleStateVariable = nullptr;
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


void Material::SetSampler(ID3D11Device* pDevice, ID3D11DeviceContext* pDeviceContext)
{
    //https://learn.microsoft.com/en-us/windows/win32/api/d3d11/ns-d3d11-d3d11_sampler_desc
    D3D11_SAMPLER_DESC desc{};
    desc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
    desc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    desc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    desc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    desc.MipLODBias = 0;
    desc.MaxAnisotropy = 0;
    desc.ComparisonFunc = D3D11_COMPARISON_NEVER;
    desc.MinLOD = 0;
    desc.MaxLOD = D3D11_FLOAT32_MAX;

    HRESULT result = pDevice->CreateSamplerState(&desc, &m_pSamplerState);
    if (FAILED(result))
    {
        std::cout << "Failed to create sampler state\n";
        return;
    }

    pDeviceContext->PSSetSamplers(0, 1, &m_pSamplerState);
    m_pSampleStateVariable->SetSampler(0, m_pSamplerState);
}

void Material::SetTexture(ID3D11Device* pDevice, ID3DX11EffectShaderResourceVariable* srv, const std::string& assetFile)
{
    if(srv)
     srv->SetResource(Texture::LoadFromFile(pDevice, assetFile)->GetSRV());
}


