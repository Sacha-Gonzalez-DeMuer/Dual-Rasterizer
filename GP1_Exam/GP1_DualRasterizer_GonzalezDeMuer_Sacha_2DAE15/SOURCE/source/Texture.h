#pragma once

class Texture
{
public:
	Texture(ID3D11Device* pDevice, SDL_Surface* pSurface);
	~Texture();

	static std::shared_ptr<Texture> LoadFromFile(ID3D11Device* pDevice, const std::string& path);

	ID3D11ShaderResourceView* GetSRV() const { return m_pShaderResourceView; };
	dae::ColorRGB Sample(const dae::Vector2& uv) const;

private:
	void Initialize(ID3D11Device* pDevice, SDL_Surface* pSurface);

	ID3D11Texture2D* m_pResource{ nullptr };
	ID3D11ShaderResourceView* m_pShaderResourceView{ nullptr };

	SDL_Surface* m_pSurface{ nullptr };
	uint32_t* m_pSurfacePixels{ nullptr };
};

