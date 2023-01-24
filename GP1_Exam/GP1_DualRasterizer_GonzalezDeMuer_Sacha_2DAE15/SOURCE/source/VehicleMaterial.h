#pragma once
#include "Material.h"

class Texture;
class Mesh;

class VehicleMaterial final : public Material
{
public:
	VehicleMaterial(ID3D11Device* pDevice, const std::wstring& assetFile);
	virtual ~VehicleMaterial() override;
	VehicleMaterial(const VehicleMaterial&) = delete;
	VehicleMaterial(VehicleMaterial&&) noexcept = delete;
	VehicleMaterial& operator=(const VehicleMaterial&) = delete;
	VehicleMaterial& operator=(VehicleMaterial&&) noexcept = delete;

	std::shared_ptr<Texture> GetDiffuseMap() const { return m_pDiffuseMap; };
	std::shared_ptr<Texture> GetNormalMap() const { return m_pNormalMap; };
	std::shared_ptr<Texture> GetSpecularMap() const { return m_pSpecularMap; };
	std::shared_ptr<Texture> GetGlossinessMap() const { return m_pGlossinessMap; };

	virtual ColorRGB PixelShading(const Vertex_Out& v, const Mesh& mesh, const Camera& camera) const override;

private:
	void SetShaderResources();

	std::shared_ptr<Texture> m_pDiffuseMap;
	std::shared_ptr<Texture> m_pNormalMap;
	std::shared_ptr<Texture> m_pSpecularMap;
	std::shared_ptr<Texture> m_pGlossinessMap;


	ID3DX11EffectShaderResourceVariable* m_pDiffuseMapVariable{ nullptr };
	ID3DX11EffectShaderResourceVariable* m_pNormalMapVariable{ nullptr };
	ID3DX11EffectShaderResourceVariable* m_pSpecularMapVariable{ nullptr };
	ID3DX11EffectShaderResourceVariable* m_pGlossinessMapVariable{ nullptr };
};

