#pragma once
#include <cassert>
#include <SDL_keyboard.h>
#include <SDL_mouse.h>

#include "Math.h"
#include "Timer.h"

namespace dae
{
	struct Camera
	{
		Camera() = default;

		Camera(const Vector3& _origin, float _fovAngle):
			origin{_origin},
			fovAngle{_fovAngle}
		{
		}


		Vector3 origin{};
		float fovAngle{90.f};
		float fov{ tanf((fovAngle * TO_RADIANS) / 2.f) };
		float nearPlane{ .1f };
		float farPlane{ 100.f };
		float aspectRatio{ 1.f };

		float lastFOV{0};
		float lastAspectRatio{0};
		float lastPitch{0};
		float lastYaw{0};
		Vector3 lastOrigin{};

		Vector3 forward{Vector3::UnitZ};
		Vector3 up{Vector3::UnitY};
		Vector3 right{Vector3::UnitX};

		float totalPitch{};
		float totalYaw{};

		Matrix invViewMatrix{};
		Matrix viewMatrix{};
		Matrix projectionMatrix{};


		float movementSpeed{ 1.0f };

		void Initialize(float _fovAngle = 90.f, Vector3 _origin = {0.f,0.f,0.f}, float _aspectRatio = 1.f)
		{
			fovAngle = _fovAngle;
			fov = tanf((fovAngle * TO_RADIANS) / 2.f);
			aspectRatio = _aspectRatio;
			origin = _origin;
		}

		void CalculateViewMatrix()
		{
			//TODO W1
			//ONB => invViewMatrix
			//Inverse(ONB) => ViewMatrix
			viewMatrix = Matrix::CreateLookAtLH(origin, forward, up);
			invViewMatrix = Matrix::Inverse(viewMatrix);
			
			//ViewMatrix => Matrix::CreateLookAtLH(...) [not implemented yet]
			//DirectX Implementation => https://learn.microsoft.com/en-us/windows/win32/direct3d9/d3dxmatrixlookatlh
		}

		void CalculateProjectionMatrix()
		{
			//TODO W2
			//ProjectionMatrix => Matrix::CreatePerspectiveFovLH(...) [not implemented yet]
			projectionMatrix = Matrix::CreatePerspectiveFovLH(fov, aspectRatio, nearPlane, farPlane);
			//DirectX Implementation => https://learn.microsoft.com/en-us/windows/win32/direct3d9/d3dxmatrixperspectivefovlh
		}

		void HandleMouseTransforms(const float constSpeed) {
			int mouseX{}, mouseY{};
			const uint32_t mouseState = SDL_GetRelativeMouseState(&mouseX, &mouseY);

			if (mouseState & SDL_BUTTON_LMASK)
			{
				if (mouseState & SDL_BUTTON_RMASK)
				{
					origin -= mouseY * constSpeed * up;
				}
				else
				{
					origin += mouseY * constSpeed * forward;
					totalYaw += mouseX * constSpeed;
				}
			}
			else if (mouseState & SDL_BUTTON_RMASK)
			{
				totalPitch -= mouseY * constSpeed;
				totalYaw += mouseX * constSpeed;
			}
		}

		void HandleKeyboardTransforms(const float constSpeed)
		{
			const uint8_t* pKeyboardState = SDL_GetKeyboardState(nullptr);
			origin += (pKeyboardState[SDL_SCANCODE_W] + -pKeyboardState[SDL_SCANCODE_S]) * constSpeed * forward;
			origin += (-pKeyboardState[SDL_SCANCODE_A] + pKeyboardState[SDL_SCANCODE_D]) * constSpeed * right;
			fov += (pKeyboardState[SDL_SCANCODE_SPACE]) * constSpeed * .3f;
			fov -= (pKeyboardState[SDL_SCANCODE_P]) * constSpeed * .3f;

		}

		void UpdatePreviousTransforms() {
			lastOrigin = origin;
			lastPitch = totalPitch;
			lastYaw = totalYaw;
		}
		
		void UpdateCamSettings() {
			lastFOV = fov;
			lastAspectRatio = aspectRatio;
		}

		void Update(Timer* pTimer)
		{
			const float constSpeed{ pTimer->GetElapsed() * movementSpeed };

			const bool isTransformUpToDate
			{
				lastPitch == totalPitch &&
				lastYaw == totalYaw &&
				lastOrigin == origin 
			};


			const bool isCamSettingsUpToDate
			{
				lastFOV == fov && lastAspectRatio == aspectRatio
			};

			//Camera Update Logic
			//...
			HandleMouseTransforms(constSpeed);
			HandleKeyboardTransforms(constSpeed);

			if (!isTransformUpToDate)
			{
				UpdatePreviousTransforms();

				Matrix finalRotation{ Matrix::CreateRotation(totalPitch, totalYaw, 0) };
				forward = finalRotation.TransformVector(Vector3::UnitZ).Normalized();
				up = finalRotation.TransformVector(Vector3::UnitY).Normalized();
				right = finalRotation.TransformVector(Vector3::UnitX).Normalized();

				//Update Matrices
				CalculateViewMatrix();
			}

			if (!isCamSettingsUpToDate) {
				UpdateCamSettings();
				CalculateProjectionMatrix(); 
			}
		}
	};
}
