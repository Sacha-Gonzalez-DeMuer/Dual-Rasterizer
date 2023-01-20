#pragma once
#include "Scene.h"
#include "MeshManager.h"
#include "VehicleMaterial.h"

class VehicleScene : public Scene
{
public:
	VehicleScene() = default;
	~VehicleScene() override = default;

	VehicleScene(const VehicleScene&) = delete;
	VehicleScene(VehicleScene&&) noexcept = delete;
	VehicleScene& operator=(const VehicleScene&) = delete;
	VehicleScene& operator=(VehicleScene&&) noexcept = delete;

	void Initialize(ID3D11Device* pDevice)
	{
		auto mesh{ AddMesh(MeshManager::Get()->GetMesh("Resources/vehicle.obj")) };
		auto pVehicleMat{ std::make_shared<VehicleMaterial>(pDevice, L"Resources/vehicle_shader.fx") };

		mesh->SetMaterial(pVehicleMat);
		if (pDevice)
			mesh->DXInitialize(pDevice);
	}
};

