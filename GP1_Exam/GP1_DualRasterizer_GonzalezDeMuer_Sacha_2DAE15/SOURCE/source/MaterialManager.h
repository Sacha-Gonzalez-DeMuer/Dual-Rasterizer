#pragma once
#include "Material.h"
#include <unordered_map>
#include "DXRenderer.h"

class MaterialManager
{
public:
	using MaterialDictionary = std::unordered_map<std::wstring, std::shared_ptr<Material>>;
	static MaterialManager* Get();
	MaterialManager();

	MaterialManager(const MaterialManager& other) = delete;
	MaterialManager(MaterialManager&& other) = delete;
	MaterialManager& operator=(const MaterialManager& other) = delete;
	MaterialManager& operator=(MaterialManager&& other) = delete;

	template <typename T>
	std::shared_ptr<T> GetMaterial(const std::wstring& filePath, ID3D11Device* pDevice = nullptr)
	{
		if (!m_MaterialDictionary.contains(filePath))
		{
			m_MaterialDictionary.insert(std::make_pair(filePath, std::make_shared<T>(pDevice, filePath)));
		}

		return std::dynamic_pointer_cast<T>(m_MaterialDictionary[filePath]);
	}

	MaterialDictionary GetMaterials() { return m_MaterialDictionary; };

private:
	MaterialDictionary m_MaterialDictionary;
};

