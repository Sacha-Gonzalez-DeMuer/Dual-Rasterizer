#pragma once
#include "Scene.h"
#include "CombustionMaterial.h"
#include "MeshMaterial.h"

class MainScene : public Scene
{
public:
	MainScene() = default;
	~MainScene() override = default;

	MainScene(const MainScene&) = delete;
	MainScene(MainScene&&) noexcept = delete;
	MainScene& operator=(const MainScene&) = delete;
	MainScene& operator=(MainScene&&) noexcept = delete;

	void Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pDeviceContext) override
	{
		auto pChassisMat{ std::make_shared<MeshMaterial>(pDevice, L"Resources/vehicle_shader.fx") };
		auto pCombustionMat{ std::make_shared<CombustionMaterial>(pDevice, L"Resources/combustion_shader.fx") };

		AddMesh(pDevice, pDeviceContext, "Resources/vehicle.obj", pChassisMat);
		AddMesh(pDevice, pDeviceContext, "Resources/fireFX.obj", pCombustionMat);
	}
};