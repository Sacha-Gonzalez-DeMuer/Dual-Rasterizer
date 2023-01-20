#pragma once
#include <vector>
#include "Camera.h"
class Mesh;
class Material;

class Scene
{
public:
	Scene();
	virtual ~Scene();
	Scene(const Scene&) = delete;
	Scene(Scene&&) noexcept = delete;
	Scene& operator=(const Scene&) = delete;
	Scene& operator=(Scene&&) noexcept = delete;

	virtual void Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pDeviceContext) = 0;
	virtual void Update(const dae::Timer* pTimer);
	virtual void Render(ID3D11Device* pDevice, ID3D11DeviceContext* pDeviceContext);

	Camera& GetCamera() { return m_Camera; };

protected:
	Camera m_Camera;

	std::vector<Mesh*> m_pMeshes{};

	void AddMesh(ID3D11Device* pDevice, ID3D11DeviceContext* pDeviceContext, const std::string& filePath, Material* mat);
};