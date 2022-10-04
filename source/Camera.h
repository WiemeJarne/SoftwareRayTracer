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
		float fovAngle{90.f};

		Vector3 forward{ Vector3::UnitZ };
		Vector3 up{Vector3::UnitY};
		Vector3 right{Vector3::UnitX};

		float totalPitch{0.f};
		float totalYaw{0.f};

		Matrix cameraToWorld{};


		Matrix CalculateCameraToWorld()
		{
			//todo: W2
			Vector3 right{ Vector3::Cross(up, forward).Normalized() };
			Vector3 up{ Vector3::Cross(forward, right).Normalized() };
			return Matrix
			{
				{right, 0},
				{up, 0},
				{forward, 0},
				{origin, 1}
			};
		}

		void Update(Timer* pTimer)
		{
			const float cameraMovementSpeed{ 20.f };
			const float cameraRoatationSpeed{ 180.f*TO_RADIANS };
			const float elapsedSec{ pTimer->GetElapsed() };

			//Keyboard Input
			const uint8_t* pKeyboardState = SDL_GetKeyboardState(nullptr);

			if (pKeyboardState[SDL_SCANCODE_W])
			{
				origin.z += cameraMovementSpeed * elapsedSec;
			}

			if (pKeyboardState[SDL_SCANCODE_S])
			{
				origin.z -= cameraMovementSpeed * elapsedSec;
			}

			if (pKeyboardState[SDL_SCANCODE_D])
			{
				origin.x += cameraMovementSpeed * elapsedSec;
			}

			if (pKeyboardState[SDL_SCANCODE_A])
			{
				origin.x -= cameraMovementSpeed * elapsedSec;
			}

			//Mouse Input
			int mouseX{}, mouseY{};
			const uint32_t mouseState = SDL_GetRelativeMouseState(&mouseX, &mouseY);

			if ((mouseState & SDL_BUTTON_RMASK) && (mouseState & SDL_BUTTON_LMASK))
			{
				if (mouseY > 0)
				{
					origin.y -= cameraMovementSpeed * elapsedSec;
				}

				if (mouseY < 0)
				{
					origin.y += cameraMovementSpeed * elapsedSec;
				}

				if (mouseX > 0)
				{
					origin.x += cameraMovementSpeed * elapsedSec;
				}

				if (mouseX < 0)
				{
					origin.x -= cameraMovementSpeed * elapsedSec;
				}
			}
			else if (mouseState & SDL_BUTTON_LMASK)
			{
				if (mouseY > 0)
				{
					origin.z -= cameraMovementSpeed * elapsedSec;
				}

				if (mouseY < 0)
				{
					origin.z += cameraMovementSpeed * elapsedSec;
				}
		
				if (mouseX > 0)
				{
					totalYaw += cameraRoatationSpeed * elapsedSec;
				}

				if (mouseX < 0)
				{
					totalYaw -= cameraRoatationSpeed * elapsedSec;
				}

				Matrix result{ Matrix::CreateRotation(0, totalYaw, 0) };
				forward = result.TransformVector(Vector3::UnitZ);
				forward.Normalize();
			}
			else if (mouseState & SDL_BUTTON_RMASK)
			{
				if (mouseX > 0)
				{
					totalYaw += cameraRoatationSpeed * elapsedSec;
				}

				if (mouseX < 0)
				{
					totalYaw -= cameraRoatationSpeed * elapsedSec;
				}

				if (mouseY > 0)
				{
					totalPitch -= cameraRoatationSpeed * elapsedSec;
				}

				if (mouseY < 0)
				{
					totalPitch += cameraRoatationSpeed * elapsedSec;
				}

				Matrix result{ Matrix::CreateRotation(totalPitch, totalYaw, 0) };
				forward = result.TransformVector(Vector3::UnitZ);
				forward.Normalize();
			}
		}
	};
}
