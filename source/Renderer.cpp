//External includes
#include "SDL.h"
#include "SDL_surface.h"

//Project includes
#include "Renderer.h"
#include "Math.h"
#include "Matrix.h"
#include "Material.h"
#include "Scene.h"
#include "Utils.h"

using namespace dae;

Renderer::Renderer(SDL_Window * pWindow) :
	m_pWindow(pWindow),
	m_pBuffer(SDL_GetWindowSurface(pWindow))
{
	//Initialize
	SDL_GetWindowSize(pWindow, &m_Width, &m_Height);
	m_pBufferPixels = static_cast<uint32_t*>(m_pBuffer->pixels);
}

void Renderer::Render(Scene* pScene) const
{
	Camera& camera = pScene->GetCamera();
	auto& materials = pScene->GetMaterials();
	auto& lights = pScene->GetLights();

	const Matrix cameraToWorld{ camera.CalculateCameraToWorld() };

	const float aspectRatio{ static_cast<float>(m_Width) / m_Height };

	const float fov{ tan(camera.fovAngle*TO_RADIANS / 2.f) };

	const size_t amountOfLights{ lights.size() };

	for (int px{}; px < m_Width; ++px)
	{
		for (int py{}; py < m_Height; ++py)
		{
			Vector3 rayDirection{};
			rayDirection.x = (2 * (px + 0.5f) / float(m_Width) - 1) * aspectRatio * fov;
			rayDirection.y = ( 1 - 2 * (py + 0.5f) / m_Height ) * fov;
			rayDirection.z = 1.f;

			rayDirection.Normalize();

			rayDirection = cameraToWorld.TransformVector(rayDirection);

			Ray ViewRay{ camera.origin, rayDirection };

			HitRecord closestHit{};

			pScene->GetClosestHit(ViewRay, closestHit);

			ColorRGB finalColor{};

			if (closestHit.didHit)
			{
				for (const Light& light : lights)
				{
					Vector3 rayOrigin{ closestHit.origin };
					rayOrigin.y += 0.0001f;
					Vector3 directionToLight{ LightUtils::GetDirectionToLight(light, rayOrigin)};
					const float distanceToLight{ directionToLight.Magnitude() };
					directionToLight.Normalize();

					Ray lightRay{ rayOrigin, directionToLight };
					lightRay.max = distanceToLight;

					if (pScene->DoesHit(lightRay))
					{
						finalColor = materials[closestHit.materialIndex]->Shade() * 0.5f;
					}
					else
					{
						finalColor = materials[closestHit.materialIndex]->Shade();
					}
				}

				//t value visulazation darker = smaller t
				//const float scaled_t = closestHit.t / 500.f;
				//finalColor = { scaled_t, scaled_t, scaled_t };
			}

			//Update Color in Buffer
			finalColor.MaxToOne();

			m_pBufferPixels[px + (py * m_Width)] = SDL_MapRGB(m_pBuffer->format,
				static_cast<uint8_t>(finalColor.r * 255),
				static_cast<uint8_t>(finalColor.g * 255),
				static_cast<uint8_t>(finalColor.b * 255));
		}
	}

	//@END
	//Update SDL Surface
	SDL_UpdateWindowSurface(m_pWindow);
}

bool Renderer::SaveBufferToImage() const
{
	return SDL_SaveBMP(m_pBuffer, "RayTracing_Buffer.bmp");
}
