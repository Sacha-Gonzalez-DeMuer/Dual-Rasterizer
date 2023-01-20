#pragma once

using namespace dae;

class Material
{
public:
	Material(ID3D11Device* pDevice, const std::wstring& assetFile);
	virtual ~Material();
	Material(const Material&) = delete;
	Material(Material&&) noexcept = delete;
	Material& operator=(const Material&) = delete;
	Material& operator=(Material&&) noexcept = delete;

	ID3DX11EffectTechnique* GetTechnique() const { return m_pTechnique; };
	ID3DX11EffectMatrixVariable* GetWorldViewProjectionMatrix() const { return m_pWorldViewProjMax; };
	                  
	void UpdateEffect(const Matrix& worldMatrix, const Matrix& invViewMatrix, const Matrix& worldViewProjection);
	void SetSampler(ID3D11Device* pDevice, ID3D11DeviceContext* pDeviceContext);
	virtual void SetTexture(ID3D11Device* pDevice, ID3DX11EffectShaderResourceVariable* srv, const std::string& assetFile);

protected:
	ID3DX11Effect* m_pEffect{ nullptr };

private:
	static ID3DX11Effect* LoadEffect(ID3D11Device* pDevice, const std::wstring& assetFile);
	void SetMatrices();

	void SetWorldViewProjectionMatrix(const Matrix& worldViewProjectionMatrix);
	void SetWorldMatrix(const Matrix& worldMatrix);
	void SetInvViewMatrix(const Matrix& invViewMatrix);

	ID3DX11EffectTechnique* m_pTechnique{ nullptr };

	ID3DX11EffectMatrixVariable* m_pWorldViewProjMax{ nullptr };
	ID3DX11EffectMatrixVariable* m_pWorldMatrix{ nullptr };
	ID3DX11EffectMatrixVariable* m_pInvViewMatrix{ nullptr };


	ID3D11SamplerState* m_pSamplerState{ nullptr };
	ID3DX11EffectSamplerVariable* m_pSampleStateVariable{ nullptr };
};