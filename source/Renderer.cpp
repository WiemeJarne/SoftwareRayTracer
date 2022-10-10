//External includes
#include "SDL.h"
#include "SDL_surface.h"

//Project includes
#include "Renderer.h"
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

	const float floatWidth{ static_cast<float>(m_Width) };
	const float floatHeight{ static_cast<float>(m_Height) };

	const float aspectRatio{ floatWidth / floatHeight };

	const float fov{ tan(camera.fovAngle * TO_RADIANS / 2.f) };

	const size_t amountOfLights{ lights.size() };

	Vector3 rayDirection{};
	Ray viewRay{};
	HitRecord closestHit{};
	ColorRGB finalColor{};

	for (int px{}; px < m_Width; ++px)
	{
		for (int py{}; py < m_Height; ++py)
		{
			rayDirection.x = (2 * (px + 0.5f) / floatWidth - 1) * aspectRatio * fov;
			rayDirection.y = ( 1 - 2 * (py + 0.5f) / floatHeight ) * fov;
			rayDirection.z = 1.f;
			rayDirection.Normalize();
			rayDirection = cameraToWorld.TransformVector(rayDirection);

			viewRay = { camera.origin, rayDirection };

			closestHit = {};
			pScene->GetClosestHit(viewRay, closestHit);

			finalColor = {};

			if (closestHit.didHit)
			{
				for (const Light& light : lights)
				{
					Vector3 rayOrigin{ closestHit.origin };
					rayOrigin += closestHit.normal * 0.0001f;

					Vector3 toLight{ LightUtils::GetDirectionToLight(light, rayOrigin) };
					const float distanceToLight = toLight.Normalize();

					if(m_ShadowsEnabled)
					{
						Ray lightRay{};
						lightRay.origin = rayOrigin;
						lightRay.direction = toLight;
						lightRay.max = distanceToLight;

						if (!pScene->DoesHit(lightRay))
						{
							CalculateFinalColor(closestHit, toLight, materials, light, rayDirection, finalColor);
						}
					}
					else
					{
						CalculateFinalColor(closestHit, toLight, materials, light, rayDirection, finalColor);
					}
					
				}

				//t value visualization darker = smaller t
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

void Renderer::CycleLightingMode()
{
	switch (m_CurrentLightingMode)
	{
	case LightingMode::ObservedArea:
		m_CurrentLightingMode = LightingMode::Radiance;
		break;
	case LightingMode::Radiance:
		m_CurrentLightingMode = LightingMode::BRFD;
		break;
	case LightingMode::BRFD:
		m_CurrentLightingMode = LightingMode::Combined;
		break;
	case LightingMode::Combined:
		m_CurrentLightingMode = LightingMode::ObservedArea;
		break;
	}
}

void Renderer::CalculateFinalColor(const HitRecord& closestHit, const Vector3& toLight, const std::vector<Material*>& materials, const Light& light, const Vector3& rayDirection, ColorRGB& finalColor) const
{
	const float observedArea{ Vector3::Dot(closestHit.normal, toLight) };

	switch (m_CurrentLightingMode)
	{
	case LightingMode::Combined:
		if (observedArea > 0)
		{
			finalColor += LightUtils::GetRadiance(light, closestHit.origin) * materials[closestHit.materialIndex]->Shade(closestHit, toLight, rayDirection) * observedArea;
		}
		break;

	case LightingMode::Radiance:
		finalColor += LightUtils::GetRadiance(light, closestHit.origin);
		break;

	case LightingMode::BRFD:
		finalColor += materials[closestHit.materialIndex]->Shade(closestHit, toLight, rayDirection);
		break;

	case LightingMode::ObservedArea:
		if (observedArea > 0)
		{
			finalColor += materials[closestHit.materialIndex]->Shade(closestHit, toLight, rayDirection) * observedArea;
		}
		break;
	}
}