#pragma once
#include <cassert>
#include <SDL_keyboard.h>
#include <SDL_mouse.h>

#include "Math.h"
#include "Timer.h"

#include <iostream>

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
		float fovAngle{ 90.f };
		const float maxFovAngle{ 179.f };
		const float minFovAngle{ 1.f };

		Vector3 forward{ Vector3::UnitZ };
		Vector3 up{Vector3::UnitY};
		Vector3 right{Vector3::UnitX};

		float totalPitch{0.f};
		float totalYaw{0.f};

		Matrix cameraToWorld{};

		Matrix CalculateCameraToWorld()
		{
			const Vector3 right{ Vector3::Cross(up, forward).Normalized() };
			const Vector3 up{ Vector3::Cross(forward, right).Normalized() };
			cameraToWorld =
			{
				{right, 0},
				{up, 0},
				{forward, 0},
				{origin, 1}
			};
			return cameraToWorld;
		}

		void Update(Timer* pTimer)
		{
			float cameraMovementSpeed{ 20.f };
			float cameraRotationSpeed{ 180.f*TO_RADIANS };
			const float elapsedSec{ pTimer->GetElapsed() };

			//Keyboard Input
			const uint8_t* pKeyboardState = SDL_GetKeyboardState(nullptr);

			if (pKeyboardState[SDL_SCANCODE_LSHIFT])
			{
				cameraMovementSpeed *= 4;
				cameraRotationSpeed *= 4;
			}

			if (pKeyboardState[SDL_SCANCODE_W] || pKeyboardState[SDL_SCANCODE_UP])
			{
				origin += forward * cameraMovementSpeed * elapsedSec;
			}

			if (pKeyboardState[SDL_SCANCODE_S] || pKeyboardState[SDL_SCANCODE_DOWN])
			{
				origin += -forward * cameraMovementSpeed * elapsedSec;
			}

			if (pKeyboardState[SDL_SCANCODE_D] || pKeyboardState[SDL_SCANCODE_RIGHT])
			{
				origin += right * cameraMovementSpeed * elapsedSec;
			}

			if (pKeyboardState[SDL_SCANCODE_A] || pKeyboardState[SDL_SCANCODE_LEFT])
			{
				origin += -right * cameraMovementSpeed * elapsedSec;
			}

			//change fov
			if (pKeyboardState[SDL_SCANCODE_Q] && fovAngle > minFovAngle)
			{
				fovAngle -= 1.f;
			}

			if (pKeyboardState[SDL_SCANCODE_E] && fovAngle < maxFovAngle)
			{
				fovAngle += 1.f;
			}

			//Mouse Input
			int mouseX{}, mouseY{};
			const uint32_t mouseState = SDL_GetRelativeMouseState(&mouseX, &mouseY);

			if ((mouseState & SDL_BUTTON_RMASK) && (mouseState & SDL_BUTTON_LMASK))
			{
				if (mouseY > 0)
				{
					origin += -up * cameraMovementSpeed * elapsedSec;
				}

				if (mouseY < 0)
				{
					origin += up * cameraMovementSpeed * elapsedSec;
				}

				if (mouseX > 0)
				{
					origin += right * cameraMovementSpeed * elapsedSec;
				}

				if (mouseX < 0)
				{
					origin += -right * cameraMovementSpeed * elapsedSec;
				}
			}
			else if (mouseState & SDL_BUTTON_LMASK)
			{
				if (mouseY > 0)
				{
					origin += forward * -cameraMovementSpeed * elapsedSec;
				}

				if (mouseY < 0)
				{
					origin += forward * cameraMovementSpeed * elapsedSec;
				}
		
				if (mouseX > 0)
				{
					totalYaw += cameraRotationSpeed * elapsedSec;
				}

				if (mouseX < 0)
				{
					totalYaw -= cameraRotationSpeed * elapsedSec;
				}

				const Matrix result{ Matrix::CreateRotation(totalPitch, totalYaw, 0) };
				forward = result.TransformVector(Vector3::UnitZ);
				forward.Normalize();
				right = result.TransformVector(Vector3::UnitX);
				right.Normalize();
			}
			else if (mouseState & SDL_BUTTON_RMASK)
			{
				if (mouseX > 0)
				{
					totalYaw += cameraRotationSpeed * elapsedSec;
				}

				if (mouseX < 0)
				{
					totalYaw -= cameraRotationSpeed * elapsedSec;
				}

				if (mouseY > 0)
				{
					totalPitch -= cameraRotationSpeed * elapsedSec;
				}

				if (mouseY < 0)
				{
					totalPitch += cameraRotationSpeed * elapsedSec;
				}

				const Matrix result{ Matrix::CreateRotation(totalPitch, totalYaw, 0) };
				forward = result.TransformVector(Vector3::UnitZ);
				forward.Normalize();
				right = result.TransformVector(Vector3::UnitX);
				right.Normalize();
			}
		}
	};
}
