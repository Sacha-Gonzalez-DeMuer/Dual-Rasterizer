#include "pch.h"
#include "Effect.h"
#include "Matrix.h"
#include "Texture.h"

Effect::Effect(ID3D11Device* pDevice, const std::wstring& assetFile)
    :m_pEffect{ LoadEffect(pDevice, assetFile) }
{
    m_pTechnique = m_pEffect->GetTechniqueByName("DefaultTechnique");
    if (!m_pTechnique->IsValid())
        std::wcout << L"Technique not valid\n";

    m_pMatWorldViewProjVariable = m_pEffect->GetVariableByName("gWorldViewProj")->AsMatrix();
    if (!m_pMatWorldViewProjVariable->IsValid())
    {
        std::wcout << L"m_pMatWorldViewProjVariable not valid!\n";
    }

    m_pDiffuseMapVariable = m_pEffect->GetVariableByName("gDiffuseMap")->AsShaderResource();
    if (!m_pDiffuseMapVariable->IsValid())
    {
        std::wcout << L"m_pDiffuseMapVariable not valid!\n";
    }

    m_pSampleStateVariable = m_pEffect->GetVariableByName("gSamplePoint")->AsSampler();
}

Effect::~Effect()
{
    m_pTechnique->Release();
    m_pTechnique = nullptr;

    m_pEffect->Release();
    m_pEffect = nullptr;

    m_pInputLayout->Release();
    m_pInputLayout = nullptr;

    m_pMatWorldViewProjVariable->Release();
    m_pMatWorldViewProjVariable = nullptr;

    m_pDiffuseMapVariable->Release();
    m_pDiffuseMapVariable = nullptr;
}

void Effect::SetMatrix(const Matrix& worldViewProjectionMatrix)
{
    m_pMatWorldViewProjVariable->SetMatrix(reinterpret_cast<const float*>(&worldViewProjectionMatrix));
}

void Effect::SetDiffuseMap(Texture* pDiffuseTexture)
{
    if (m_pDiffuseMapVariable)
        m_pDiffuseMapVariable->SetResource(pDiffuseTexture->GetSRV());
}

void Effect::SetSampler(ID3D11Device* pDevice, ID3D11DeviceContext* pDeviceContext)
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

ID3DX11Effect* Effect::LoadEffect(ID3D11Device* pDevice, const std::wstring& assetFile)
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
