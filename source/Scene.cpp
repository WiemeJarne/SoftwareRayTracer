#include "Scene.h"
#include "Utils.h"
#include "Material.h"

namespace dae {

#pragma region Base Scene
	Scene::Scene()
	{
		m_SphereGeometries.reserve(32);
		m_PlaneGeometries.reserve(32);
		m_TriangleMeshGeometries.reserve(32);
		m_Lights.reserve(32);
	}

	Scene::~Scene()
	{
		for (auto& pMaterial : m_SolidColorMaterials)
		{
			delete pMaterial;
			pMaterial = nullptr;
		}
		m_SolidColorMaterials.clear();

		for (auto& pMaterial : m_LambertMaterials)
		{
			delete pMaterial;
			pMaterial = nullptr;
		}
		m_LambertMaterials.clear();

		for (auto& pMaterial : m_LambertPhongMaterials)
		{
			delete pMaterial;
			pMaterial = nullptr;
		}
		m_LambertPhongMaterials.clear();

		for (auto& pMaterial : m_CookTorrenceMaterials)
		{
			delete pMaterial;
			pMaterial = nullptr;
		}
		m_CookTorrenceMaterials.clear();
	}

	void dae::Scene::GetClosestHit(const Ray& ray, HitRecord& closestHit) const
	{
		const size_t amountOfSpheres{ m_SphereGeometries.size() };
		for (size_t index{}; index < amountOfSpheres; ++index)
		{
			GeometryUtils::HitTest_Sphere(m_SphereGeometries[index], ray, closestHit);

		}

		const size_t amountOfPlanes{ m_PlaneGeometries.size() };
		for (size_t index{}; index < amountOfPlanes; ++index)
		{
			GeometryUtils::HitTest_Plane(m_PlaneGeometries[index], ray, closestHit);
		}

		if (GeometryUtils::SlabTest_TriangleMesh(m_AABB.min, m_AABB.max, ray))
		{
			const size_t amountOfTrianglesMeshes{ m_TriangleMeshGeometries.size() };
			for (size_t index{}; index < amountOfTrianglesMeshes; ++index)
			{
				GeometryUtils::HitTest_TriangleMesh(m_TriangleMeshGeometries[index], ray, closestHit);
			}
		}
	}

	bool Scene::DoesHit(const Ray& ray) const
	{
		//todo W3
		HitRecord closestHit{};

		const size_t amountOfSpheres{ m_SphereGeometries.size() };
		for (size_t index{}; index < amountOfSpheres; ++index)
		{
			if (GeometryUtils::HitTest_Sphere(m_SphereGeometries[index], ray, closestHit, true))
			{
				return true;
			}
		}

		const size_t amountOfPlanes{ m_PlaneGeometries.size() };
		for (size_t index{}; index < amountOfPlanes; ++index)
		{
			if (GeometryUtils::HitTest_Plane(m_PlaneGeometries[index], ray, closestHit, true))
			{
				return true;
			}
		}

		if (GeometryUtils::SlabTest_TriangleMesh(m_AABB.min, m_AABB.max, ray))
		{
			const size_t amountOfTriangles{ m_TriangleMeshGeometries.size() };
			for (size_t index{}; index < amountOfTriangles; ++index)
			{
				if (GeometryUtils::HitTest_TriangleMesh(m_TriangleMeshGeometries[index], ray, closestHit, true))
				{
					return true;
				}
			}
		}
		return false;
	}

#pragma region Scene Helpers
	Sphere* Scene::AddSphere(const Vector3& origin, float radius, MaterialType materialType, unsigned char materialIndex)
	{
		Sphere s;
		s.origin = origin;
		s.radius = radius;
		s.materialIndex = materialIndex;
		s.materialType = materialType;

		m_SphereGeometries.emplace_back(s);
		return &m_SphereGeometries.back();
	}

	Plane* Scene::AddPlane(const Vector3& origin, const Vector3& normal, MaterialType materialType, unsigned char materialIndex)
	{
		Plane p;
		p.origin = origin;
		p.normal = normal;
		p.materialIndex = materialIndex;
		p.materialType = materialType;

		m_PlaneGeometries.emplace_back(p);
		return &m_PlaneGeometries.back();
	}

	TriangleMesh* Scene::AddTriangleMesh(TriangleCullMode cullMode, MaterialType materialType, unsigned char materialIndex)
	{
		TriangleMesh m{};
		m.cullMode = cullMode;
		m.materialIndex = materialIndex;
		m.materialType = materialType;

		m_TriangleMeshGeometries.emplace_back(m);
		return &m_TriangleMeshGeometries.back();
	}

	Light* Scene::AddPointLight(const Vector3& origin, float intensity, const ColorRGB& color)
	{
		Light l;
		l.origin = origin;
		l.intensity = intensity;
		l.color = color;
		l.type = LightType::Point;

		m_Lights.emplace_back(l);
		return &m_Lights.back();
	}

	Light* Scene::AddDirectionalLight(const Vector3& direction, float intensity, const ColorRGB& color)
	{
		Light l;
		l.direction = direction;
		l.intensity = intensity;
		l.color = color;
		l.type = LightType::Directional;

		m_Lights.emplace_back(l);
		return &m_Lights.back();
	}

	unsigned char Scene::AddMaterialSolidColor(Material_SolidColor* pMaterial)
	{
		m_SolidColorMaterials.push_back(pMaterial);
		return static_cast<unsigned char>(m_SolidColorMaterials.size() - 1);
	}

	unsigned char Scene::AddMaterialLambert(Material_Lambert* pMaterial)
	{
		m_LambertMaterials.push_back(pMaterial);
		return static_cast<unsigned char>(m_LambertMaterials.size() - 1);
	}

	unsigned char Scene::AddMaterialLambertPhong(Material_LambertPhong* pMaterial)
	{
		m_LambertPhongMaterials.push_back(pMaterial);
		return static_cast<unsigned char>(m_LambertPhongMaterials.size() - 1);
	}

	unsigned char Scene::AddMaterialCookTorrence(Material_CookTorrence* pMaterial)
	{
		m_CookTorrenceMaterials.push_back(pMaterial);
		return static_cast<unsigned char>(m_CookTorrenceMaterials.size() - 1);
	}
#pragma endregion
#pragma endregion

#pragma region SCENE W1
	void Scene_W1::Initialize()
	{
		const unsigned char matId_Solid_Red = AddMaterialSolidColor(new Material_SolidColor{ colors::Red });
		const unsigned char matId_Solid_Blue = AddMaterialSolidColor(new Material_SolidColor{ colors::Blue });

		const unsigned char matId_Solid_Yellow = AddMaterialSolidColor(new Material_SolidColor{ colors::Yellow });
		const unsigned char matId_Solid_Green = AddMaterialSolidColor(new Material_SolidColor{ colors::Green });
		const unsigned char matId_Solid_Magenta = AddMaterialSolidColor(new Material_SolidColor{ colors::Magenta });

		//Spheres
		AddSphere({ -25.f, 0.f, 100.f }, 50.f, MaterialType::solidColor, matId_Solid_Red);
		AddSphere({ 25.f, 0.f, 100.f }, 50.f, MaterialType::solidColor, matId_Solid_Blue);

		//Plane
		AddPlane({ -75.f, 0.f, 0.f }, { 1.f, 0.f, 0.f }, MaterialType::solidColor, matId_Solid_Green);
		AddPlane({ 75.f, 0.f, 0.f }, { -1.f, 0.f, 0.f }, MaterialType::solidColor, matId_Solid_Green);
		AddPlane({ 0.f, -75.f, 0.f }, { 0.f, 1.f, 0.f }, MaterialType::solidColor, matId_Solid_Yellow);
		AddPlane({ 0.f, 75.f, 0.f }, { 0.f, -1.f, 0.f }, MaterialType::solidColor, matId_Solid_Yellow);
		AddPlane({ 0.f, 0.f, 125.f }, { 0.f, 0.f,-1.f }, MaterialType::solidColor, matId_Solid_Magenta);
	}
#pragma endregion

#pragma region SCENE W2
	void Scene_W2::Initialize()
	{
		m_Camera.origin = { 0.f, 3.f, -9.f };
		m_Camera.fovAngle = 45.f;

		const unsigned char matId_Solid_Red = AddMaterialSolidColor(new Material_SolidColor{ colors::Red });
		const unsigned char matId_Solid_Blue = AddMaterialSolidColor(new Material_SolidColor{ colors::Blue });

		const unsigned char matId_Solid_Yellow = AddMaterialSolidColor(new Material_SolidColor{ colors::Yellow });
		const unsigned char matId_Solid_Green = AddMaterialSolidColor(new Material_SolidColor{ colors::Green });
		const unsigned char matId_Solid_Magenta = AddMaterialSolidColor(new Material_SolidColor{ colors::Magenta });

		//Plane
		AddPlane({ -5.f, 0.f, 0.f }, { 1.f, 0.f, 0.f }, MaterialType::solidColor, matId_Solid_Green);
		AddPlane({ 5.f, 0.f, 0.f }, { -1.f, 0.f, 0.f }, MaterialType::solidColor, matId_Solid_Green);
		AddPlane({ 0.f, 0.f, 0.f }, { 0.f, 1.f, 0.f }, MaterialType::solidColor, matId_Solid_Yellow);
		AddPlane({ 0.f, 10.f, 0.f }, { 0.f, -1.f, 0.f }, MaterialType::solidColor, matId_Solid_Yellow);
		AddPlane({ 0.f, 0.f, 10.f }, { 0.f, 0.f, -1.f }, MaterialType::solidColor, matId_Solid_Magenta);

		//Spheres
		AddSphere({ -1.75f, 1.f, 0.f }, .75f, MaterialType::solidColor, matId_Solid_Red);
		AddSphere({ 0.f, 1.f, 0.f }, .75f, MaterialType::solidColor, matId_Solid_Blue);
		AddSphere({ 1.75f, 1.f, 0.f }, .75f, MaterialType::solidColor, matId_Solid_Red);
		AddSphere({ -1.75f, 3.f, 0.f }, .75f, MaterialType::solidColor, matId_Solid_Blue);
		AddSphere({ 0.f, 3.f, 0.f }, .75f, MaterialType::solidColor, matId_Solid_Red);
		AddSphere({ 1.75f, 3.f, 0.f }, .75f, MaterialType::solidColor, matId_Solid_Blue);

		//Light
		AddPointLight({ 0.f, 5.f, -5.f }, 70.f, colors::White);
	}
#pragma endregion

#pragma region SCENE W3
	void Scene_W3::Initialize()
	{
		m_Camera.origin = { 0.f, 3.f, -9.f };
		m_Camera.fovAngle = 45.f;

		const auto matCT_GrayRoughMetal{ AddMaterialCookTorrence(new Material_CookTorrence({.972f, .960f, .915f}, 1.f, 1.f)) };
		const auto matCT_GrayMediumMetal{ AddMaterialCookTorrence(new Material_CookTorrence({.972f, .960f, .915f}, 1.f, .6f)) };
		const auto matCT_GraySmoothMetal{ AddMaterialCookTorrence(new Material_CookTorrence({.972f, .960f, .915f}, 1.f, .1f)) };
		const auto matCT_GrayRoughPlastic{ AddMaterialCookTorrence(new Material_CookTorrence({.75f, .75f, .75f}, .0f, 1.f)) };
		const auto matCT_GrayMediumPlastic{ AddMaterialCookTorrence(new Material_CookTorrence({.75f, .75f, .75f}, .0f, .6f)) };
		const auto matCT_GraySmoothPlastic{ AddMaterialCookTorrence(new Material_CookTorrence({.75f, .75f, .75f}, .0f, .1f)) };

		const auto matLambert_GrayBlue{ AddMaterialLambert(new Material_Lambert({.49f, .57f, .57f}, 1.f)) };

		//lambert-Phong spheres and materials for testing
		//const auto matLambertPhong1 = AddMaterialLambertPhong(new Material_LambertPhong(colors::Blue, 0.5f, 0.5f, 3.f));
		//const auto matLambertPhong2 = AddMaterialLambertPhong(new Material_LambertPhong(colors::Blue, 0.5f, 0.5f, 15.f));
		//const auto matLambertPhong3 = AddMaterialLambertPhong(new Material_LambertPhong(colors::Blue, 0.5f, 0.5f, 50.f));

		//AddSphere(Vector3{ -1.75f, 1.f, 0.f }, .75f, MaterialType::lambertPhong, matLambertPhong1);
		//AddSphere(Vector3{ 0.f, 1.f, 0.f }, .75f, MaterialType::lambertPhong, matLambertPhong2);
		//AddSphere(Vector3{ 1.75f, 1.f, 0.f }, .75f, MaterialType::lambertPhong, matLambertPhong3);

		//Planes
		AddPlane(Vector3{ 0.f, 0.f, 10.f }, Vector3{ 0.f, 0.f, -1.f }, MaterialType::lambertPhong, matLambert_GrayBlue); //back
		AddPlane(Vector3{ 0.f, 0.f, 0.f }, Vector3{ 0.f, 1.f, 0.f }, MaterialType::lambertPhong, matLambert_GrayBlue); //bottom
		AddPlane(Vector3{ 0.f, 10.f, 0.f }, Vector3{ 0.f, -1.f, 0.f }, MaterialType::lambertPhong, matLambert_GrayBlue); //top
		AddPlane(Vector3{ 5.f, 0.f, 0.f }, Vector3{ -1.f, 0.f, 0.f }, MaterialType::lambertPhong, matLambert_GrayBlue); //right
		AddPlane(Vector3{ -5.f, 0.f, 0.f }, Vector3{ 1.f, 0.f, 0.f }, MaterialType::lambertPhong, matLambert_GrayBlue); //left

		//Spheres
		AddSphere(Vector3{ -1.75, 1.f, 0.f }, .75f, MaterialType::cookTorrence, matCT_GrayRoughMetal);
		AddSphere(Vector3{ 0.f, 1.f, 0.f }, .75f, MaterialType::cookTorrence, matCT_GrayMediumMetal);
		AddSphere(Vector3{ 1.75, 1.f, 0.f }, .75f, MaterialType::cookTorrence, matCT_GraySmoothMetal);
		AddSphere(Vector3{ -1.75, 3.f, 0.f }, .75f, MaterialType::cookTorrence, matCT_GrayRoughPlastic);
		AddSphere(Vector3{ 0.f, 3.f, 0.f }, .75f, MaterialType::cookTorrence, matCT_GrayMediumPlastic);
		AddSphere(Vector3{ 1.75, 3.f, 0.f }, .75f, MaterialType::cookTorrence, matCT_GraySmoothPlastic);

		//Lights
		AddPointLight(Vector3{ 0.f, 5.f, 5.f }, 50.f, ColorRGB{ 1.f, .61f, .45f }); //Back Light
		AddPointLight(Vector3{ -2.5f, 5.f, -5.f }, 70.f, ColorRGB{ 1.f, .8f, .45f }); //Front Left Light
		AddPointLight(Vector3{ 2.5f, 2.5f, -5.f }, 50.f, ColorRGB{ .34f, .47f, .68f });
	}
#pragma endregion

#pragma region SCENE W3 TESTSCENE
	void Scene_W3_TestScene::Initialize()
	{
		m_Camera.origin = { 0.f, 1.f, -5.f };
		m_Camera.fovAngle = 45.f;

		const unsigned char matId_Solid_Red = AddMaterialSolidColor(new Material_SolidColor{ colors::Red });
		const unsigned char matId_Solid_Blue = AddMaterialSolidColor(new Material_SolidColor{ colors::Blue });
		const unsigned char matId_Solid_Yellow = AddMaterialSolidColor(new Material_SolidColor{ colors::Yellow });

		//Lambert Materials
		//const auto matLambert_Red = AddMaterialLambert(new Material_Lambert{ colors::Red, 1.f });
		//const auto matLambert_Blue = AddMaterialLambert(new Material_Lambert{ colors::Blue, 1.f });
		//const auto matLambert_Yellow = AddMaterialLambert(new Material_Lambert{ colors::Yellow, 1.f });

		//Phong Material
		const auto matLambertPhong_Blue = AddMaterialLambertPhong(new Material_LambertPhong(colors::Blue, 1.f, 1.f, 6.f));

		//Spheres
		AddSphere({ -.75f, 1.f, .0f }, 1.f, MaterialType::solidColor, matId_Solid_Red);
		AddSphere({ .75f, 1.f, .0f }, 1.f, MaterialType::solidColor, matLambertPhong_Blue);

		//Plane
		AddPlane({ 0.f, 0.f, 0.f }, { 0.f, 1.f, 0.f }, MaterialType::solidColor, matId_Solid_Yellow);

		//Light
		AddPointLight({ 0.f, 5.f, 5.f }, 25.f, colors::White);
		AddPointLight({ 0.f, 2.5f, -5.f }, 25.f, colors::White);
	}
#pragma endregion

#pragma region SCENE W4 TESTSCENE
	void Scene_W4_TestScene::Initialize()
	{
		m_Camera.origin = { 0.f, 1.f, -5.f };
		m_Camera.fovAngle = 45.f;

		//Materials
		const auto matLambert_GrayBlue = AddMaterialLambert(new Material_Lambert({ .49f, .57f, .57f }, 1.f));
		const auto matLambert_White = AddMaterialLambert(new Material_Lambert(colors::White, 1.f));

		//planes
		AddPlane(Vector3{ 0.f, 0.f, 10.f }, Vector3{ 0.f, 0.f, -1.f }, MaterialType::lambert, matLambert_GrayBlue); //back
		AddPlane(Vector3{ 0.f, 0.f, 0.f }, Vector3{ 0.f, 1.f, 0.f }, MaterialType::lambert, matLambert_GrayBlue); //bottom
		AddPlane(Vector3{ 0.f, 10.f, 0.f }, Vector3{ 0.f, -1.f, 0.f }, MaterialType::lambert, matLambert_GrayBlue); //top
		AddPlane(Vector3{ 5.f, 0.f, 0.f }, Vector3{ -1.f, 0.f, 0.f }, MaterialType::lambert, matLambert_GrayBlue); //right
		AddPlane(Vector3{ -5.f, 0.f, 0.f }, Vector3{ 1.f, 0.f, 0.f }, MaterialType::lambert, matLambert_GrayBlue); //left

		//triangle (temp) DOES NOT WORK ANYMORE ADD m_Triangles in Scene.h for this to work!
		//auto triangle = Triangle{ {-.75f, .5f, 0.f}, {-.75f, 2.f, 0.f}, {.75f, .5f, 0.f} };
		//triangle.cullMode = TriangleCullMode::NoCulling;
		//triangle.materialIndex = matLambert_White;
		//
		//m_Triangles.emplace_back(triangle);

		//Triangle Mesh
		pMesh = AddTriangleMesh(TriangleCullMode::BackFaceCulling, MaterialType::lambert, matLambert_White);

		Utils::ParseOBJ("Resources/simple_object.obj",
			pMesh->positions,
			pMesh->normals,
			pMesh->indices);

		pMesh->Scale({ .7f, .7f, .7f });
		pMesh->Translate({ 0.f, 1.f, 0.f });

		pMesh->UpdateTransforms();

		//Lights
		AddPointLight(Vector3{ 0.f, 5.f, 5.f }, 50.f, ColorRGB{ 1.f, .61f, .45f }); //backLight
		AddPointLight(Vector3{ -2.5f, 5.f, -5.f }, 70.f, ColorRGB{ 1.f, .8f, .45f }); //Front Light left
		AddPointLight(Vector3{ 2.5f, 2.5f, -5.f }, 50.f, ColorRGB{ .34f, .47f, .68f });
	}

	void Scene_W4_TestScene::Update(Timer* pTimer)
	{
		Scene::Update(pTimer);

		pMesh->RotateY(PI_DIV_2 * pTimer->GetTotal());
		pMesh->UpdateTransforms();
	}
#pragma endregion

#pragma region SCENE W4 REFERENCESCENE
	void Scene_W4_ReferenceScene::Initialize()
	{
		sceneName = "Reference Scene";
		m_Camera.origin = { 0, 3, -9 };
		m_Camera.fovAngle = 45.f;

		const auto matCT_GrayRoughMetal{ AddMaterialCookTorrence(new Material_CookTorrence({.972f, .960f, .915f}, 1.f, 1.f)) };
		const auto matCT_GrayMediumMetal{ AddMaterialCookTorrence(new Material_CookTorrence({.972f, .960f, .915f}, 1.f, .6f)) };
		const auto matCT_GraySmoothMetal{ AddMaterialCookTorrence(new Material_CookTorrence({.972f, .960f, .915f}, 1.f, .1f)) };
		const auto matCT_GrayRoughPlastic{ AddMaterialCookTorrence(new Material_CookTorrence({.75f, .75f, .75f}, .0f, 1.f)) };
		const auto matCT_GrayMediumPlastic{ AddMaterialCookTorrence(new Material_CookTorrence({.75f, .75f, .75f}, .0f, .6f)) };
		const auto matCT_GraySmoothPlastic{ AddMaterialCookTorrence(new Material_CookTorrence({.75f, .75f, .75f}, .0f, .1f)) };

		const auto matLambert_GrayBlue{ AddMaterialLambert(new Material_Lambert({.49f, .57f, .57f}, 1.f)) };
		const auto matLambert_White{ AddMaterialLambert(new Material_Lambert(colors::White, 1.f)) };

		//Planes
		AddPlane(Vector3{ 0.f, 0.f, 10.f }, Vector3{ 0.f, 0.f, -1.f }, MaterialType::lambert, matLambert_GrayBlue); //back
		AddPlane(Vector3{ 0.f, 0.f, 0.f }, Vector3{ 0.f, 1.f, 0.f }, MaterialType::lambert, matLambert_GrayBlue); //bottom
		AddPlane(Vector3{ 0.f, 10.f, 0.f }, Vector3{ 0.f, -1.f, 0.f }, MaterialType::lambert, matLambert_GrayBlue); //top
		AddPlane(Vector3{ 5.f, 0.f, 0.f }, Vector3{ -1.f, 0.f, 0.f }, MaterialType::lambert, matLambert_GrayBlue); //right
		AddPlane(Vector3{ -5.f, 0.f, 0.f }, Vector3{ 1.f, 0.f, 0.f }, MaterialType::lambert, matLambert_GrayBlue); //left

		//Spheres
		AddSphere(Vector3{ -1.75, 1.f, 0.f }, .75f, MaterialType::cookTorrence, matCT_GrayRoughMetal);
		AddSphere(Vector3{ 0.f, 1.f, 0.f }, .75f, MaterialType::cookTorrence, matCT_GrayMediumMetal);
		AddSphere(Vector3{ 1.75, 1.f, 0.f }, .75f, MaterialType::cookTorrence, matCT_GraySmoothMetal);
		AddSphere(Vector3{ -1.75, 3.f, 0.f }, .75f, MaterialType::cookTorrence, matCT_GrayRoughPlastic);
		AddSphere(Vector3{ 0.f, 3.f, 0.f }, .75f, MaterialType::cookTorrence, matCT_GrayMediumPlastic);
		AddSphere(Vector3{ 1.75, 3.f, 0.f }, .75f, MaterialType::cookTorrence, matCT_GraySmoothPlastic);

		//TriangleMesh
		const Triangle baseTriangle{ Vector3{-0.75f, 1.5f, 0.f}, Vector3{.75f, 0.f, 0.f}, Vector3{-.75f, 0.f, 0.f} };

		m_Meshes[0] = AddTriangleMesh(TriangleCullMode::BackFaceCulling, MaterialType::lambert, matLambert_White);
		m_Meshes[0]->AppendTriangle(baseTriangle, true);
		m_Meshes[0]->Translate({ -1.75f, 4.5f, 0.f });
		m_Meshes[0]->UpdateAABB();
		m_Meshes[0]->UpdateTransforms();

		m_Meshes[1] = AddTriangleMesh(TriangleCullMode::FrontFaceCulling, MaterialType::lambert, matLambert_White);
		m_Meshes[1]->AppendTriangle(baseTriangle, true);
		m_Meshes[1]->Translate({ 0.f, 4.5f, 0.f });
		m_Meshes[1]->UpdateAABB();
		m_Meshes[1]->UpdateTransforms();

		m_Meshes[2] = AddTriangleMesh(TriangleCullMode::NoCulling, MaterialType::lambert, matLambert_White);
		m_Meshes[2]->AppendTriangle(baseTriangle, true);
		m_Meshes[2]->Translate({ 1.75f, 4.5f, 0.f });
		m_Meshes[2]->UpdateAABB();
		m_Meshes[2]->UpdateTransforms();

		//Lights
		AddPointLight(Vector3{ 0.f, 5.f, 5.f }, 50.f, ColorRGB{ 1.f, .61f, .45f }); //Back Light
		AddPointLight(Vector3{ -2.5f, 5.f, -5.f }, 70.f, ColorRGB{ 1.f, .8f, .45f }); //Front Left Light
		AddPointLight(Vector3{ 2.5f, 2.5f, -5.f }, 50.f, ColorRGB{ .34f, .47f, .68f });
	}

	void Scene_W4_ReferenceScene::Update(Timer* pTimer)
	{
		Scene::Update(pTimer);

		const auto yawAngle = (cos(pTimer->GetTotal()) + 1.f) / 2.f * PI_2;
		for (const auto m : m_Meshes)
		{
			m->RotateY(yawAngle);
			m->UpdateTransforms();
			m_AABB.grow(m->transformedMinAABB);
			m_AABB.grow(m->transformedMaxAABB);
		}
	}
#pragma endregion

#pragma region SCENE W4 BUNNYSCENE
	void Scene_W4_BunnyScene::Initialize()
	{
		m_Camera.origin = { 0, 3, -9 };
		m_Camera.fovAngle = 45.f;

		//Material
		const auto matLambert_GrayBlue = AddMaterialLambert(new Material_Lambert({ .49f, .57f, .57f }, 1.f));
		const auto matLambert_White = AddMaterialLambert(new Material_Lambert(colors::White, 1.f));

		//planes
		AddPlane(Vector3{ 0.f, 0.f, 10.f }, Vector3{ 0.f, 0.f, -1.f }, MaterialType::lambert, matLambert_GrayBlue); //back
		AddPlane(Vector3{ 0.f, 0.f, 0.f }, Vector3{ 0.f, 1.f, 0.f }, MaterialType::lambert, matLambert_GrayBlue); //bottom
		AddPlane(Vector3{ 0.f, 10.f, 0.f }, Vector3{ 0.f, -1.f, 0.f }, MaterialType::lambert, matLambert_GrayBlue); //top
		AddPlane(Vector3{ 5.f, 0.f, 0.f }, Vector3{ -1.f, 0.f, 0.f }, MaterialType::lambert, matLambert_GrayBlue); //right
		AddPlane(Vector3{ -5.f, 0.f, 0.f }, Vector3{ 1.f, 0.f, 0.f }, MaterialType::lambert, matLambert_GrayBlue); //left

		//Bunny Mesh
		m_pMesh = AddTriangleMesh(TriangleCullMode::BackFaceCulling, MaterialType::lambert, matLambert_White);

		Utils::ParseOBJ("Resources/lowpoly_bunny2.obj",
			m_pMesh->positions,
			m_pMesh->normals,
			m_pMesh->indices);

		m_pMesh->Scale({ 2.f, 2.f, 2.f });

		m_pMesh->UpdateAABB();
		m_pMesh->UpdateTransforms();

		//Lights
		AddPointLight(Vector3{ 0.f, 5.f, 5.f }, 50.f, ColorRGB{ 1.f, .61f, .45f }); //Back Light
		AddPointLight(Vector3{ -2.5f, 5.f, -5.f }, 70.f, ColorRGB{ 1.f, .8f, .45f }); //Front Left Light
		AddPointLight(Vector3{ 2.5f, 2.5f, -5.f }, 50.f, ColorRGB{ .34f, .47f, .68f });
	}

#pragma endregion
}