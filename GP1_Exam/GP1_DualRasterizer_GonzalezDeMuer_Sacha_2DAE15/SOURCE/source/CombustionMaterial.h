#pragma once
#include "Material.h"

class CombustionMaterial final : public Material
{
public:
	CombustionMaterial(ID3D11Device* pDevice, const std::wstring& assetFile);
	virtual ~CombustionMaterial() override;
	CombustionMaterial(const CombustionMaterial&) = delete;
	CombustionMaterial(CombustionMaterial&&) noexcept = delete;
	CombustionMaterial& operator=(const CombustionMaterial&) = delete;
	CombustionMaterial& operator=(CombustionMaterial&&) noexcept = delete;


	std::shared_ptr<Texture> GetDiffuseMap() const { return m_pDiffuseMap; };
	virtual ColorRGB PixelShading(const Vertex_Out& v, const Mesh& mesh, const Camera& camera) const override;

private:
	void SetShaderResources();

	std::shared_ptr<Texture>  m_pDiffuseMap;
	ID3DX11EffectShaderResourceVariable* m_pDiffuseMapVariable{ nullptr };
};