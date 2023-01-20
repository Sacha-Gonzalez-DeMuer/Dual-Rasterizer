#include "pch.h"
#include "CombustionMaterial.h"
#include "Texture.h"

CombustionMaterial::CombustionMaterial(ID3D11Device* pDevice, const std::wstring& assetFile)
    : Material(pDevice, assetFile)
{
    SetShaderResources();

    SetTexture(pDevice, m_pDiffuseMapVariable, "Resources/fireFX_diffuse.png");
}

CombustionMaterial::~CombustionMaterial()
{
    m_pDiffuseMapVariable->Release();
    m_pDiffuseMapVariable = nullptr;
}


void CombustionMaterial::SetShaderResources()
{
    // Diffuse
    m_pDiffuseMapVariable = m_pEffect->GetVariableByName("gDiffuseMap")->AsShaderResource();
    if (!m_pDiffuseMapVariable->IsValid())
        std::wcout << L"m_pDiffuseMapVariable not valid!\n";
}
