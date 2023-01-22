#include "pch.h"
#include "VehicleMaterial.h"
#include "Matrix.h"
#include "Texture.h"
#include "Mesh.h"
#include "Camera.h"
#include "BRDFs.h"
#include "FilePaths.h"

VehicleMaterial::VehicleMaterial(ID3D11Device* pDevice, const std::wstring& assetFile)
    : Material(pDevice, assetFile)
{
    SetShaderResources();

    m_pDiffuseMap = SetTexture(pDevice, m_pDiffuseMapVariable, FILE_TEX_VEHICLE_DIFFUSE);
    m_pNormalMap = SetTexture(pDevice, m_pNormalMapVariable, FILE_TEX_VEHICLE_NORMAL);
    m_pSpecularMap = SetTexture(pDevice, m_pSpecularMapVariable, FILE_TEX_VEHICLE_SPECULAR);
    m_pGlossinessMap = SetTexture(pDevice, m_pGlossinessMapVariable, FILE_TEX_VEHICLE_GLOSS);
}

ColorRGB VehicleMaterial::PixelShading(const Vertex_Out& v, const Mesh& mesh, const Camera& camera) const
{
	ColorRGB finalColor{};
	const Vector3 lightDirection{ .577f, -.577f, .577f };
	constexpr float lightIntensity{ 7.f };

	constexpr float specularShininess{ 25.f };
	const float diffuseReflectance{ 1.f };
	const float specular{ m_pSpecularMap->Sample(v.uv).r };
	const float phongExponent{ m_pGlossinessMap->Sample(v.uv).g * specularShininess };
	Vector3 viewDirection{ camera.GetInvViewMatrix().TransformVector(camera.GetForward()).Normalized()};

	//create tangent space transformation matrix
	const Vector3 binormal{ Vector3::Cross(v.normal, v.tangent).Normalized() };
	const Matrix tangentSpace{ -v.tangent, binormal, v.normal, Vector3::Zero };

	//sample normal
	const ColorRGB sampledNormal{ (2.f * m_pNormalMap->Sample(v.uv) - ColorRGB(1,1,1)) }; //to [-1, 1]
	const Vector3 vSampledNormal{ Vector3(sampledNormal.r, sampledNormal.g, sampledNormal.b) };
	const Vector3 tangentSpaceSampledNormal{ tangentSpace.TransformVector(vSampledNormal).Normalized() };

	//choose which normal to use
	Vector3 usedNormal{ m_UseNormalMap ? tangentSpaceSampledNormal : v.normal };

	const float observedArea{ std::max(0.f, Vector3::Dot(usedNormal, -lightDirection)) };

	//BRDFs
	const ColorRGB lambertDiffuse{ BRDF::Lambert(diffuseReflectance, m_pDiffuseMap->Sample(v.uv)) * lightIntensity };
	const ColorRGB phongSpecular{ BRDF::Phong(specular, phongExponent, lightDirection, -viewDirection, usedNormal) };

	constexpr ColorRGB ambient{ .025f, .025f, .025f };

	switch (m_ShadingMode)
	{
	case ShadingMode::ObservedArea:
		finalColor += ColorRGB(1, 1, 1) * observedArea;
		break;
	case ShadingMode::Diffuse:
		finalColor += lambertDiffuse * observedArea;
		break;
	case ShadingMode::Specular:
		finalColor += ColorRGB(1, 1, 1) * phongSpecular * observedArea;
		break;
	default:
	case ShadingMode::Combined:
		finalColor += lambertDiffuse * observedArea + phongSpecular + ambient;
	}

	finalColor.MaxToOne();

	return finalColor;
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
