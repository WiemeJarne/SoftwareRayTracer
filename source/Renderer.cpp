//External includes
#include "SDL.h"
#include "SDL_surface.h"
#include <future> //async
#include <ppl.h> //parallel_for

//Project includes
#include "Renderer.h"
#include "Matrix.h"
#include "Material.h"
#include "Scene.h"
#include "Utils.h"

using namespace dae;

//#define ASYNC
#define PARALLEL_FOR

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
	camera.CalculateCameraToWorld();

	const float fov{ tan(camera.fovAngle * TO_RADIANS / 2.f) };
	
	const float aspectRatio{ static_cast<float>(m_Width) / static_cast<float>(m_Height) };

	auto& materials = pScene->GetMaterials();
	auto& lights = pScene->GetLights();

	const uint32_t amountOfPixels = m_Width * m_Height;

#if defined(ASYNC)
	//Async Logic
	const uint32_t amountOfCores{ std::thread::hardware_concurrency() };
	std::vector<std::future<void>> async_futures{};
	const uint32_t amountOfPixelsPerTask{ amountOfPixels / amountOfCores };
	uint32_t amountOfUnassignedPixels{ amountOfPixels % amountOfCores };
	uint32_t currentPixelIndex{};

	for (uint32_t coreId{ 0 }; coreId < amountOfCores; ++coreId)
	{
		uint32_t taskSize{ amountOfPixelsPerTask };
		if (amountOfUnassignedPixels > 0)
		{
			++taskSize;
			--amountOfUnassignedPixels;
		}

		async_futures.push_back(std::async(std::launch::async, [=, this]
			{
				//render all pixels for this task (currentPixelIndex > currentPixelIndex + taskSize)
				const uint32_t lastPixelIndex{ currentPixelIndex + taskSize };
				for (uint32_t pixelIndex{ currentPixelIndex }; pixelIndex < lastPixelIndex; ++pixelIndex)
				{
					RenderPixel(pScene, pixelIndex, fov, aspectRatio, camera, lights, materials);
				}
			}));

		currentPixelIndex += taskSize;
	}

	//wait for async completion of all tasks
	for (const std::future<void>& f : async_futures)
	{
		f.wait();
	}

#elif defined(PARALLEL_FOR)
	//Parallel-For logic
	concurrency::parallel_for(0u, amountOfPixels, [=, this](int index)
		{
			RenderPixel(pScene, index, fov, aspectRatio, camera, lights, materials);
		});

#else
	//Synchronous Logic (no threading)
	for (uint32_t index{}; index < amountOfPixel; ++index)
	{
		RenderPixel(pScene, index, fov, aspectRatio, pScene->GetCamera(), lights, materials);
	}

#endif
	
	//@END
	//Update SDL Surface
	SDL_UpdateWindowSurface(m_pWindow);
}

void Renderer::RenderPixel(Scene* pScene, uint32_t pixelIndex, float fov, float aspectRatio, const Camera& camera, const std::vector<Light>& lights, const std::vector<Material*>& materials) const
{
	const int px = pixelIndex % m_Width;
	const int py = pixelIndex / m_Width;

	Vector3 rayDirection{};
	rayDirection.x = (2 * (px + 0.5f) / static_cast<float>(m_Width) - 1) * aspectRatio * fov;
	rayDirection.y = (1 - 2 * (py + 0.5f) / static_cast<float>(m_Height)) * fov;
	rayDirection.z = 1.f;
	rayDirection.Normalize();
	rayDirection = camera.cameraToWorld.TransformVector(rayDirection);
	
	Ray viewRay{ camera.origin, rayDirection };

	HitRecord closestHit{};
	pScene->GetClosestHit(viewRay, closestHit);

	ColorRGB finalColor{};

	if (closestHit.didHit)
	{
		for (const Light& light : lights)
		{
			Vector3 rayOrigin{ closestHit.origin };
			rayOrigin += closestHit.normal * 0.0001f;

			Vector3 toLight{ LightUtils::GetDirectionToLight(light, rayOrigin) };
			const float distanceToLight = toLight.Normalize();

			if (m_ShadowsEnabled)
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
	}

	//Update Color in Buffer
	finalColor.MaxToOne();

	m_pBufferPixels[px + (py * m_Width)] = SDL_MapRGB(m_pBuffer->format,
		static_cast<uint8_t>(finalColor.r * 255),
		static_cast<uint8_t>(finalColor.g * 255),
		static_cast<uint8_t>(finalColor.b * 255));
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
		if (observedArea > 0.f)
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
		if (observedArea > 0.f)
		{
			finalColor += ColorRGB{ 1.f, 1.f, 1.f } * observedArea;
		}
		break;
	}
}