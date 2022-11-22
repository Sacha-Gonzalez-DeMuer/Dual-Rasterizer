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

		Vector3 forward{Vector3::UnitZ};
		Vector3 up{Vector3::UnitY};
		Vector3 right{Vector3::UnitX};

		float totalPitch{};
		float totalYaw{};

		Matrix invViewMatrix{};
		Matrix viewMatrix{};

		float movementSpeed{ 1.0f };

		void Initialize(float _fovAngle = 90.f, Vector3 _origin = {0.f,0.f,0.f})
		{
			fovAngle = _fovAngle;
			fov = tanf((fovAngle * TO_RADIANS) / 2.f);

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
		}

		void Update(Timer* pTimer)
		{
			const float deltaTime = pTimer->GetElapsed();
			const float constSpeed{ deltaTime * movementSpeed };

			//Camera Update Logic
			//...
			HandleMouseTransforms(constSpeed);
			HandleKeyboardTransforms(constSpeed);

			Matrix finalRotation{ Matrix::CreateRotation(totalPitch, totalYaw, 0) };
			forward = finalRotation.TransformVector(Vector3::UnitZ).Normalized();
			up = finalRotation.TransformVector(Vector3::UnitY).Normalized();
			right = finalRotation.TransformVector(Vector3::UnitX).Normalized();


			//Update Matrices
			CalculateViewMatrix();
			CalculateProjectionMatrix(); //Try to optimize this - should only be called once or when fov/aspectRatio changes
		}
	};
}
