#pragma once
#include "Material.h"

class Texture;

class CombustionMaterial final : public Material
{
public:
	CombustionMaterial(ID3D11Device* pDevice, const std::wstring& assetFile);
	~CombustionMaterial();
	CombustionMaterial(const CombustionMaterial&) = delete;
	CombustionMaterial(CombustionMaterial&&) noexcept = delete;
	CombustionMaterial& operator=(const CombustionMaterial&) = delete;
	CombustionMaterial& operator=(CombustionMaterial&&) noexcept = delete;

private:
	void SetShaderResources();

	ID3DX11EffectShaderResourceVariable* m_pDiffuseMapVariable{ nullptr };
};

