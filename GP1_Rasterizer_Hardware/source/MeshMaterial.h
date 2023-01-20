#pragma once
#include "Material.h"

class Texture;

class MeshMaterial final : public Material
{
public:
	MeshMaterial(ID3D11Device* pDevice, const std::wstring& assetFile);
	virtual ~MeshMaterial() override;
	MeshMaterial(const MeshMaterial&) = delete;
	MeshMaterial(MeshMaterial&&) noexcept = delete;
	MeshMaterial& operator=(const MeshMaterial&) = delete;
	MeshMaterial& operator=(MeshMaterial&&) noexcept = delete;

private:
	void SetShaderResources();


	ID3DX11EffectShaderResourceVariable* m_pDiffuseMapVariable{ nullptr };
	ID3DX11EffectShaderResourceVariable* m_pNormalMapVariable{ nullptr };
	ID3DX11EffectShaderResourceVariable* m_pSpecularMapVariable{ nullptr };
	ID3DX11EffectShaderResourceVariable* m_pGlossinessMapVariable{ nullptr };
};

