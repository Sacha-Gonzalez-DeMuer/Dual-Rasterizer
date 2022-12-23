#pragma once
using namespace dae;

class Camera final
{
public:
	Camera();
	Camera(const Vector3& origin, float fovAngle, float aspectRatio);

	void Update(const Timer* pTimer);

	Matrix GetViewMatrix() const { return m_ViewMatrix; };
	Matrix GetProjectionMatrix() const { return m_ProjectionMatrix; };

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
	Matrix m_ViewMatrix;
	Matrix m_InvViewMatrix;
	Matrix m_ProjectionMatrix;

	//Member functions
	//===
	void CalculateViewMatrix();
	void CalculateProjectionMatrix();
	void HandleMouseTransforms(const float constSpeed);
	void HandleKeyboardTransforms(const float constSpeed);
};

