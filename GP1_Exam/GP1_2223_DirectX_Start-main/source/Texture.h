#pragma once
//#include <SDL_surface.h>
//#include <SDL_image.h>
//#include <string>

class Texture
{
public:
	Texture(ID3D11Device* pDevice, SDL_Surface* pSurface);
	~Texture();

	static std::unique_ptr<Texture> LoadFromFile(ID3D11Device* pDevice, const std::string& path);

	ID3D11ShaderResourceView* GetSRV() const { return m_pShaderResourceView; };
private:
	void Initialize(ID3D11Device* pDevice, SDL_Surface* pSurface);

	ID3D11Texture2D* m_pResource{ nullptr };
	ID3D11ShaderResourceView* m_pShaderResourceView{ nullptr };
};

