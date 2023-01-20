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

	// In initialize the required objects should be added to our vector
	virtual void Initialize(ID3D11Device* pDevice = nullptr) = 0;
	void Update(const dae::Timer* pTimer);

	virtual void SRender();
	virtual void DXRender(ID3D11Device* pDevice, ID3D11DeviceContext* pDeviceContext);

	Camera& GetCamera() { return m_Camera; };

protected:
	Camera m_Camera;
	std::shared_ptr<Mesh> AddMesh(std::shared_ptr<Mesh> pMesh);

private:
	std::vector<std::shared_ptr<Mesh>> m_pMeshes{};
};