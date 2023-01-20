#include "pch.h"
#include "VehicleMaterial.h"
#include "Matrix.h"
#include "Texture.h"

VehicleMaterial::VehicleMaterial(ID3D11Device* pDevice, const std::wstring& assetFile)
    : Material(pDevice, assetFile)
{
    SetShaderResources();

    m_pDiffuseMap = SetTexture(pDevice, m_pDiffuseMapVariable, "Resources/vehicle_diffuse.png");
    m_pNormalMap = SetTexture(pDevice, m_pNormalMapVariable, "Resources/vehicle_normal.png");
    m_pSpecularMap = SetTexture(pDevice, m_pSpecularMapVariable, "Resources/vehicle_specular.png");
    m_pGlossinessMap = SetTexture(pDevice, m_pGlossinessMapVariable, "Resources/vehicle_gloss.png");
}

void VehicleMaterial::SetShaderResources()
{
    // Diffuse
    m_pDiffuseMapVariable = m_pEffect->GetVariableByName("gDiffuseMap")->AsShaderResource();
    if (!m_pDiffuseMapVariable->IsValid())
        std::wcout << L"m_pDiffuseMapVariable not valid!\n";

    // Normal
    m_pNormalMapVariable = m_pEffect->GetVariableByName("gNormalMap")->AsShaderResource();
    if (!m_pNormalMapVariable->IsValid())
        std::wcout << L"m_pNormalMapVariable not valid!\n";

    // Specular
    m_pSpecularMapVariable = m_pEffect->GetVariableByName("gSpecularMap")->AsShaderResource();
    if (!m_pSpecularMapVariable->IsValid())
        std::wcout << L"m_pSpecularMapVariable not valid!\n";

    // Glossiness
    m_pGlossinessMapVariable = m_pEffect->GetVariableByName("gGlossinessMap")->AsShaderResource();
    if (!m_pGlossinessMapVariable->IsValid())
        std::wcout << L"m_pGlossinessMapVariable not valid!\n";
}


VehicleMaterial::~VehicleMaterial()
{
    m_pDiffuseMapVariable->Release();
    m_pDiffuseMapVariable = nullptr;

    m_pNormalMapVariable->Release();
    m_pNormalMapVariable = nullptr;

    m_pSpecularMapVariable->Release();
    m_pSpecularMapVariable = nullptr;

    m_pGlossinessMapVariable->Release();
    m_pGlossinessMapVariable = nullptr;
}
