#pragma once

using namespace dae;

class Camera final
{
public:
	Camera();
	Camera(const Vector3& origin, float fovAngle, float aspectRatio);

	void Update(const dae::Timer* pTimer);

	void SetAspectRatio(float aspectRatio) { m_AspectRatio = aspectRatio; };

	dae::Matrix GetViewMatrix() const { return m_ViewMatrix; };
	dae::Matrix GetInvViewMatrix() const { return m_InvViewMatrix; };
	dae::Matrix GetProjectionMatrix() const { return m_ProjectionMatrix; };
	Vector3 GetForward() const { return m_Forward; };

	void BoostMovementSpeed();

private:
	//camera settings
	Vector3 m_Origin;
	float m_FovAngle;
	float m_FOV;
	float m_AspectRatio;
	float m_NearPlane;
	float m_FarPlane;
	float m_MoveSpeed;

	//transformation components
	Vector3 m_Forward;
	Vector3 m_Right;
	Vector3 m_Up;
	float m_TotalPitch;
	float m_TotalYaw;
	
	//transformation matrices
	dae::Matrix m_ViewMatrix;
	dae::Matrix m_InvViewMatrix;
	dae::Matrix m_ProjectionMatrix;

	//Member functions
	//===
	void CalculateViewMatrix();
	void CalculateProjectionMatrix();
	void HandleMouseTransforms(const float constSpeed);
	void HandleKeyboardTransforms(const float constSpeed);
};

