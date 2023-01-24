#include "pch.h"
#include "CombustionMaterial.h"
#include "BRDFs.h"
#include "FilePaths.h"

CombustionMaterial::CombustionMaterial(ID3D11Device* pDevice, const std::wstring& assetFile)
    : Material(pDevice, assetFile)
{
    SetShaderResources();

    m_pDiffuseMap = SetTexture(pDevice, m_pDiffuseMapVariable, FILE_TEX_FIREFX_DIFFUSE);
}

CombustionMaterial::~CombustionMaterial()
{
    m_pDiffuseMapVariable->Release();
    m_pDiffuseMapVariable = nullptr;
}

ColorRGB CombustionMaterial::PixelShading(const Vertex_Out& v, const Mesh& mesh, const Camera& camera) const
{
    const float diffuseReflectance{ 1.f };
    constexpr float lightIntensity{ 7.f };

    const ColorRGB lambertDiffuse{ BRDF::Lambert(diffuseReflectance, m_pDiffuseMap->Sample(v.uv)) * lightIntensity };

    return lambertDiffuse;
}

void CombustionMaterial::SetShaderResources()
{
    // Diffuse
    m_pDiffuseMapVariable = m_pEffect->GetVariableByName("gDiffuseMap")->AsShaderResource();
    if (!m_pDiffuseMapVariable->IsValid())
        std::wcout << L"m_pDiffuseMapVariable not valid!\n";
}