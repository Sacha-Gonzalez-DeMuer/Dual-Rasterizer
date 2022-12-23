#pragma once
using namespace dae;
class Texture;
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
	ID3DX11EffectMatrixVariable* GetWorldViewProjectionMatrix() const { return m_pMatWorldViewProjVariable; };


	void SetMatrix(const Matrix& worldViewProjectionMatrix);
	void SetDiffuseMap(Texture* pDiffuseTexture);
	void SetSampler(ID3D11Device* pDevice, ID3D11DeviceContext* pDeviceContext);

private:
	static ID3DX11Effect* LoadEffect(ID3D11Device* pDevice, const std::wstring& assetFile);

	ID3DX11Effect* m_pEffect{ nullptr };
	ID3D11InputLayout* m_pInputLayout{ nullptr };
	ID3DX11EffectTechnique* m_pTechnique{ nullptr };

	ID3DX11EffectMatrixVariable* m_pMatWorldViewProjVariable{ nullptr };

	//2D Texture members
	Texture* m_pTexture{};

	ID3DX11EffectShaderResourceVariable* m_pDiffuseMapVariable{ nullptr };
	ID3D11SamplerState* m_pSamplerState{ nullptr };
	ID3DX11EffectSamplerVariable* m_pSampleStateVariable{ nullptr };
};

