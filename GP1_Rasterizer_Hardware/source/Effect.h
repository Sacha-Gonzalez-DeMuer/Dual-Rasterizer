#pragma once
class Effect final
{
public:
	Effect(ID3D11Device* pDevice, const std::wstring& assetFile);
	
	~Effect();
	Effect(const Effect&) = delete;
	Effect(Effect&&) noexcept = delete;
	Effect& operator=(const Effect&) = delete;
	Effect& operator=(Effect&&) noexcept = delete;

	ID3DX11EffectTechnique* GetTechnique() const { return m_pTechnique; };
	ID3D11InputLayout* GetInputLayout() const { return m_pInputLayout; };

private:
	static ID3DX11Effect* LoadEffect(ID3D11Device* pDevice, const std::wstring& assetFile);

	ID3DX11Effect* m_pEffect{ nullptr };
	ID3D11InputLayout* m_pInputLayout{ nullptr };
	ID3DX11EffectTechnique* m_pTechnique{ nullptr };
};

