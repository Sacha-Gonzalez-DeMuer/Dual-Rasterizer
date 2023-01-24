#pragma once
#include "Scene.h"
#include "MeshManager.h"
#include "MaterialManager.h"
#include "VehicleMaterial.h"
#include "CombustionMaterial.h"
#include "FilePaths.h"

class VehicleScene : public Scene
{
public:
	VehicleScene() = default;
	virtual ~VehicleScene() = default;

	VehicleScene(const VehicleScene&) = delete;
	VehicleScene(VehicleScene&&) noexcept = delete;
	VehicleScene& operator=(const VehicleScene&) = delete;
	VehicleScene& operator=(VehicleScene&&) noexcept = delete;

	void Initialize(ID3D11Device* pDevice)
	{
		// Get needed meshes
		auto pVehicleMesh{ AddMesh(MeshManager::Get()->GetMesh(FILE_OBJ_VEHICLE)) };
		auto pCombustionMesh{ AddMesh(MeshManager::Get()->GetMesh(FILE_OBJ_FIREFX)) };

		// Get needed materials
		//auto pVehicleMat{ std::make_shared<VehicleMaterial>(pDevice, FILE_FX_VEHICLE) };
		//auto pCombustionMat{ std::make_shared<CombustionMaterial>(pDevice, FILE_FX_FIRE) };

		auto pVehicleMat{ MaterialManager::Get()->GetMaterial<VehicleMaterial>(FILE_FX_VEHICLE, pDevice) };
		auto pCombustionMat{ MaterialManager::Get()->GetMaterial<CombustionMaterial>(FILE_FX_FIRE, pDevice) };


		// Assign materials meshes
		pVehicleMesh->SetMaterial(pVehicleMat);
		pCombustionMesh->SetMaterial(pCombustionMat);

		// Initialize DirectX
		if (pDevice)
		{
			pVehicleMesh->DXInitialize(pDevice);
			pCombustionMesh->DXInitialize(pDevice);
		}

		// Initialize Software rendering
		pVehicleMesh->SInitialize();
	}
};

