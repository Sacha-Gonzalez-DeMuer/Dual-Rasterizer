#include "pch.h"
#include "Camera.h"


Camera::Camera()
	: m_Origin{ 0.f, 0.f, 0.f }
	, m_FovAngle{ 45.f }
	, m_FOV{ tanf((m_FovAngle * TO_RADIANS) / 2.f) }
	, m_AspectRatio{1.f}
	, m_Forward{ Vector3::UnitZ }
	, m_Right{ Vector3::UnitX }
	, m_Up{ Vector3::UnitY }
	, m_NearPlane{.1f}
	, m_FarPlane{100.f}
	, m_MoveSpeed{1.f}
	, m_TotalPitch{ 0.f }
	, m_TotalYaw{ 0.f }
{	
	CalculateViewMatrix();
	CalculateProjectionMatrix(); 
}

Camera::Camera(const Vector3& origin, float fovAngle, float aspectRatio)
	: m_Origin{origin}
	, m_FovAngle{fovAngle}
	, m_FOV{ tanf((m_FovAngle * TO_RADIANS) / 2.f) }
	, m_AspectRatio{aspectRatio}
	, m_NearPlane{.1f}
	, m_FarPlane{100.f}
	, m_Forward{Vector3::UnitZ}
	, m_Right{Vector3::UnitX}
	, m_Up{Vector3::UnitY}
	, m_MoveSpeed{1.f}
	, m_TotalPitch{0.f}
	, m_TotalYaw{0.f}
{
	CalculateViewMatrix();
	CalculateProjectionMatrix();
}

void Camera::Update(const Timer* pTimer)
{
	const float constSpeed{ pTimer->GetElapsed() * m_MoveSpeed };

	//handle camera control
	HandleMouseTransforms(constSpeed);
	HandleKeyboardTransforms(constSpeed);

	//update axes according to pitch/yaw
	Matrix finalRotation{ Matrix::CreateRotation(m_TotalPitch, m_TotalYaw, 0) };
	m_Forward = finalRotation.TransformVector(Vector3::UnitZ).Normalized();
	m_Up = finalRotation.TransformVector(Vector3::UnitY).Normalized();
	m_Right = finalRotation.TransformVector(Vector3::UnitX).Normalized();

	//recalculate matrices with new axes
	CalculateViewMatrix();
	CalculateProjectionMatrix();
}

void Camera::CalculateViewMatrix()
{
	m_ViewMatrix = Matrix::CreateLookAtLH(m_Origin, m_Forward, m_Up);
	m_InvViewMatrix = Matrix::Inverse(m_ViewMatrix);
}

void Camera::CalculateProjectionMatrix()
{
	m_ProjectionMatrix = Matrix::CreatePerspectiveFovLH(m_FOV, m_AspectRatio, m_NearPlane, m_FarPlane);
}

void Camera::HandleMouseTransforms(const float constSpeed) {
	int mouseX{}, mouseY{};
	const uint32_t mouseState = SDL_GetRelativeMouseState(&mouseX, &mouseY);

	if (mouseState & SDL_BUTTON_LMASK)
	{
		if (mouseState & SDL_BUTTON_RMASK)
		{
			m_Origin -= mouseY * constSpeed * m_Up;
		}
		else
		{
			m_Origin += mouseY * constSpeed * m_Forward;
			m_TotalYaw += mouseX * constSpeed;
		}
	}
	else if (mouseState & SDL_BUTTON_RMASK)
	{
		m_TotalPitch -= mouseY * constSpeed;
		m_TotalYaw += mouseX * constSpeed;
	}
}

void Camera::HandleKeyboardTransforms(const float constSpeed)
{
	const uint8_t* pKeyboardState = SDL_GetKeyboardState(nullptr);
	m_Origin += (pKeyboardState[SDL_SCANCODE_W] + -pKeyboardState[SDL_SCANCODE_S]) * constSpeed * m_Forward;
	m_Origin += (-pKeyboardState[SDL_SCANCODE_A] + pKeyboardState[SDL_SCANCODE_D]) * constSpeed * m_Right;
}