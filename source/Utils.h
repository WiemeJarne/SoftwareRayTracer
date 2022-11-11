#pragma once
#include <cassert>
#include <fstream>
#include "Math.h"
#include "DataTypes.h"

namespace dae
{
	namespace GeometryUtils
	{
#pragma region Sphere HitTest
		//SPHERE HIT-TESTS
		inline bool HitTest_Sphere(const Sphere& sphere, const Ray& ray, HitRecord& hitRecord, bool ignoreHitRecord = false)
		{
			const Vector3 sphereToRayOriginVector{ ray.origin - sphere.origin };
			const float A{ Vector3::Dot(ray.direction, ray.direction) };
			const float B{ 2 * Vector3::Dot(ray.direction, sphereToRayOriginVector) };
			const float C{ Vector3::Dot(sphereToRayOriginVector, sphereToRayOriginVector) - sphere.radius * sphere.radius };

			const float discriminant{ B * B - 4 * A * C };

			if (discriminant > 0)
			{
				const float sqrtDiscriminant{ sqrtf(discriminant) };

				const float t0{ (-B - sqrtDiscriminant) / (2.f * A) };

				if (t0 > ray.min && t0 < ray.max && t0 < hitRecord.t)
				{
					if (ignoreHitRecord) return true;

					hitRecord.didHit = true;
					hitRecord.materialIndex = sphere.materialIndex;
					hitRecord.origin = ray.origin + t0 * ray.direction;
					hitRecord.normal = (hitRecord.origin - sphere.origin).Normalized();
					hitRecord.t = t0;
					hitRecord.materialType = sphere.materialType;

					return true;
				}

				const float t1{ (-B + sqrtDiscriminant) / (2.f * A) };

				if (t1 > ray.min && t1 < ray.max && t1 < hitRecord.t)
				{
					if (ignoreHitRecord) return true;

					hitRecord.didHit = true;
					hitRecord.materialIndex = sphere.materialIndex;
					hitRecord.origin = ray.origin + t1 * ray.direction;
					hitRecord.normal = (hitRecord.origin - sphere.origin).Normalized();
					hitRecord.t = t1;
					hitRecord.materialType = sphere.materialType;

					return true;
				}
			}

			return false;
		}

		inline bool HitTest_Sphere(const Sphere& sphere, const Ray& ray)
		{
			HitRecord temp{};
			return HitTest_Sphere(sphere, ray, temp, true);
		}
#pragma endregion

#pragma region Plane HitTest
		//PLANE HIT-TESTS
		inline bool HitTest_Plane(const Plane& plane, const Ray& ray, HitRecord& hitRecord, bool ignoreHitRecord = false)
		{
			//todo W1
			const float t = Vector3::Dot(plane.origin - ray.origin, plane.normal) / Vector3::Dot(ray.direction, plane.normal);

			if (t >= ray.min && t <= ray.max && t < hitRecord.t)
			{
				if (ignoreHitRecord) return true;

				hitRecord.didHit = true;
				hitRecord.materialIndex = plane.materialIndex;
				hitRecord.normal = plane.normal;
				hitRecord.origin = ray.origin + t * ray.direction;
				hitRecord.t = t;
				hitRecord.materialType = plane.materialType;

				return true;
			}

			return  false;
		}

		inline bool HitTest_Plane(const Plane& plane, const Ray& ray)
		{
			HitRecord temp{};
			return HitTest_Plane(plane, ray, temp, true);
		}
#pragma endregion

#pragma region Triangle HitTest
		//TRIANGLE HIT-TESTS
		inline bool HitTest_Triangle(const Triangle& triangle, const Ray& ray, HitRecord& hitRecord, bool ignoreHitRecord = false)
		{
			const float normalDotViewRay{ Vector3::Dot(triangle.normal, ray.direction) };

			if (ignoreHitRecord)
			{
				if (triangle.cullMode == TriangleCullMode::BackFaceCulling && normalDotViewRay < 0) return false;

				if (triangle.cullMode == TriangleCullMode::FrontFaceCulling && normalDotViewRay > 0) return false;
			}
			else
			{
				if (triangle.cullMode == TriangleCullMode::BackFaceCulling && normalDotViewRay > 0) return false;

				if (triangle.cullMode == TriangleCullMode::FrontFaceCulling && normalDotViewRay < 0) return false;
			}

			const Vector3 v0MinusV1{ triangle.v0 - triangle.v1 };
			const Vector3 v0MinusV2{ triangle.v0 - triangle.v2 };

			Matrix A{ Vector3{v0MinusV1.x, v0MinusV2.x, ray.direction.x},
					  Vector3{v0MinusV1.y, v0MinusV2.y, ray.direction.y},
					  Vector3{v0MinusV1.z, v0MinusV2.z, ray.direction.z},
					  Vector3{}											 };

			const float determinantA{ A.Determinant() };

			const Vector3 v0MinusRayOrigin{ triangle.v0 - ray.origin };

			const float t{ Matrix{ Vector3{v0MinusV1.x, v0MinusV2.x, v0MinusRayOrigin.x},
								   Vector3{v0MinusV1.y, v0MinusV2.y, v0MinusRayOrigin.y},
								   Vector3{v0MinusV1.z, v0MinusV2.z, v0MinusRayOrigin.z},
								   Vector3{} }.Determinant() / determinantA				 };

			if (t < ray.min || t > ray.max || t > hitRecord.t) return false;

			const float gamma{ Matrix{ Vector3{v0MinusV1.x, v0MinusRayOrigin.x, ray.direction.x},
									   Vector3{v0MinusV1.y, v0MinusRayOrigin.y, ray.direction.y},
									   Vector3{v0MinusV1.z, v0MinusRayOrigin.z, ray.direction.z},
									   Vector3{} }.Determinant() / determinantA					 };

			if (gamma < 0 || gamma > 1) return false;

			const float beta{ Matrix{ Vector3{v0MinusRayOrigin.x, v0MinusV2.x, ray.direction.x},
									  Vector3{v0MinusRayOrigin.y, v0MinusV2.y, ray.direction.y},
									  Vector3{v0MinusRayOrigin.z, v0MinusV2.z, ray.direction.z},
									  Vector3{} }.Determinant() / determinantA					};

			if (beta < 0 || beta >(1 - gamma)) return false;

			if (ignoreHitRecord) return true;

			hitRecord.didHit = true;
			hitRecord.materialIndex = triangle.materialIndex;
			hitRecord.normal = triangle.normal;
			hitRecord.origin = ray.origin + t * ray.direction;
			hitRecord.t = t;
			hitRecord.materialType = triangle.materialType;

			return true;
		}

		inline bool HitTest_Triangle(const Triangle& triangle, const Ray& ray)
		{
			HitRecord temp{};
			return HitTest_Triangle(triangle, ray, temp, true);
		}
#pragma endregion

#pragma region TriangleMesh SlabTest
		inline bool SlabTest_TriangleMesh(Vector3 min, Vector3 max, const Ray& ray)
		{
			//Smits’ algorithm
			//source: https://www.researchgate.net/publication/220494140_An_Efficient_and_Robust_Ray-Box_Intersection_Algorithm

			const Vector3 minAABB{ min };
			const Vector3 maxAABB{ max };

			float tMin{}, tMax{};
			const float divX{ 1.f / ray.direction.x };

			if (ray.direction.x >= 0)
			{
				tMin = (minAABB.x - ray.origin.x) * divX; //multiplying twice is faster the dividing twice
				tMax = (maxAABB.x - ray.origin.x) * divX;
			}
			else
			{
				tMin = (maxAABB.x - ray.origin.x) * divX;
				tMax = (minAABB.x - ray.origin.x) * divX;
			}

			float tYMin{}, tYMax{};
			const float divY{ 1.f / ray.direction.y };

			if (ray.direction.y >= 0)
			{
				tYMin = (minAABB.y - ray.origin.y) * divY;
				tYMax = (maxAABB.y - ray.origin.y) * divY;
			}
			else
			{
				tYMin = (maxAABB.y - ray.origin.y) * divY;
				tYMax = (minAABB.y - ray.origin.y) * divY;
			}

			if (tMin > tYMax || tYMin > tMax) return false;

			if (tYMin > tMin) tMin = tYMin;

			if (tYMax < tMax) tMax = tYMax;

			float tZMin{}, tZMax{};
			const float divZ{ 1.f / ray.direction.z };

			if (ray.direction.z >= 0)
			{
				tZMin = (minAABB.z - ray.origin.z) * divZ;
				tZMax = (maxAABB.z - ray.origin.z) * divZ;
			}
			else
			{
				tZMin = (maxAABB.z - ray.origin.z) * divZ;
				tZMax = (minAABB.z - ray.origin.z) * divZ;
			}

			if (tMin > tZMax || tZMin > tMax) return false;

			if (tZMin > tMin) tMin = tZMin;

			if (tZMax < tMax) tMax = tZMax;

			return tMin < ray.max&& tMax > ray.min;
		}

		inline bool SlabTest_TriangleMesh(const TriangleMesh& mesh, const Ray& ray)
		{
			return SlabTest_TriangleMesh(mesh.transformedMinAABB, mesh.transformedMaxAABB, ray);
		}
#pragma endregion

		inline bool HitTest_BVH(const TriangleMesh& mesh, const Ray& ray, int nodeIndex, HitRecord& hitRecord, std::vector<int>& triangleIndicesToTest, bool ignoreHitRecord = false)
		{
			const BVHNode& node{ mesh.bvhNodes[nodeIndex] };

			if (!SlabTest_TriangleMesh(node.AABBMin, node.AABBMax, ray)) return false;

			if (node.amountOfMeshes != 0)
			{
				triangleIndicesToTest.push_back(nodeIndex);
			}
			else
			{
				HitTest_BVH(mesh, ray, node.leftChildIndex, hitRecord, triangleIndicesToTest, ignoreHitRecord);
				HitTest_BVH(mesh, ray, node.leftChildIndex + 1, hitRecord, triangleIndicesToTest, ignoreHitRecord); //leftChildIndex + 1 == rightChildIndex
			}
		}

#pragma region TriangeMesh HitTest
		inline bool HitTest_TriangleMesh(const TriangleMesh& mesh, const Ray& ray, HitRecord& hitRecord, bool ignoreHitRecord = false)
		{
			//slabTest
			if (!SlabTest_TriangleMesh(mesh, ray))
			{
				return false;
			}

			Triangle triangle{};
			triangle.cullMode = mesh.cullMode;
			triangle.materialIndex = mesh.materialIndex;
			triangle.materialType = mesh.materialType;

			if(mesh.useBVH)
			{
				std::vector<int> trianglesToTestIndices{};
				 HitTest_BVH(mesh, ray, mesh.rootNodeIndex, hitRecord, trianglesToTestIndices, ignoreHitRecord);

				 if (trianglesToTestIndices.empty()) return false;

				for (int index{}; index < trianglesToTestIndices.size(); ++index)
				{
					 int start{ mesh.bvhNodes[trianglesToTestIndices[index]].leftChildIndex };
					 int end{ start + mesh.bvhNodes[trianglesToTestIndices[index]].amountOfMeshes };
					 for (int index{start}; index < end; ++index)
					 {
						 triangle.normal = mesh.transformedNormals[index];
						 triangle.v0 = mesh.transformedPositions[mesh.indices[index * 3]];
						 triangle.v1 = mesh.transformedPositions[mesh.indices[index * 3 + 1]];
						 triangle.v2 = mesh.transformedPositions[mesh.indices[index * 3 + 2]];

						 if (HitTest_Triangle(triangle, ray, hitRecord, ignoreHitRecord) && ignoreHitRecord) return true;
					 }
				}	 
			}
			else
			{
				const int amountOfTriangles{ static_cast<int>(mesh.indices.size()) / 3 };
				for (int index{}; index < amountOfTriangles; ++index)
				{
					triangle.normal = mesh.transformedNormals[index];
					triangle.v0 = mesh.transformedPositions[mesh.indices[index * 3]];
					triangle.v1 = mesh.transformedPositions[mesh.indices[index * 3 + 1]];
					triangle.v2 = mesh.transformedPositions[mesh.indices[index * 3 + 2]];

					if (HitTest_Triangle(triangle, ray, hitRecord, ignoreHitRecord) && ignoreHitRecord) return true;
				}
			}

			return false;
		}

		inline bool HitTest_TriangleMesh(const TriangleMesh& mesh, const Ray& ray)
		{
			HitRecord temp{};
			return HitTest_TriangleMesh(mesh, ray, temp, true);
		}
#pragma endregion
	}

	namespace LightUtils
	{
		//Direction from target to light
		inline Vector3 GetDirectionToLight(const Light& light, const Vector3 origin)
		{
			//todo W3
			return { light.origin - origin };
		}

		inline ColorRGB GetRadiance(const Light& light, const Vector3& target)
		{
			//todo W3
			if(light.type == LightType::Point)
			{
				const Vector3 targetToLight{ GetDirectionToLight(light, target) };
				const float irradiance{ light.intensity / (targetToLight.SqrMagnitude()) };
				return{ light.color * irradiance };
			}

			if(light.type == LightType::Directional)
			{
				return{ light.color * light.intensity };
			}

			return ColorRGB{ 0.f, 0.f, 0.f };
		}
	}

	namespace Utils
	{
		//Just parses vertices and indices
#pragma warning(push)
#pragma warning(disable : 4505) //Warning unreferenced local function
		static bool ParseOBJ(const std::string& filename, std::vector<Vector3>& positions, std::vector<Vector3>& normals, std::vector<int>& indices)
		{
			std::ifstream file(filename);
			if (!file)
				return false;

			std::string sCommand;
			// start a while iteration ending when the end of file is reached (ios::eof)
			while (!file.eof())
			{
				//read the first word of the string, use the >> operator (istream::operator>>) 
				file >> sCommand;
				//use conditional statements to process the different commands	
				if (sCommand == "#")
				{
					// Ignore Comment
				}
				else if (sCommand == "v")
				{
					//Vertex
					float x, y, z;
					file >> x >> y >> z;
					positions.push_back({ x, y, z });
				}
				else if (sCommand == "f")
				{
					float i0, i1, i2;
					file >> i0 >> i1 >> i2;

					indices.push_back((int)i0 - 1);
					indices.push_back((int)i1 - 1);
					indices.push_back((int)i2 - 1);
				}
				//read till end of line and ignore all remaining chars
				file.ignore(1000, '\n');

				if (file.eof()) 
					break;
			}

			//Precompute normals
			for (uint64_t index = 0; index < indices.size(); index += 3)
			{
				uint32_t i0 = indices[index];
				uint32_t i1 = indices[index + 1];
				uint32_t i2 = indices[index + 2];

				Vector3 edgeV0V1 = positions[i1] - positions[i0];
				Vector3 edgeV0V2 = positions[i2] - positions[i0];
				Vector3 normal = Vector3::Cross(edgeV0V1, edgeV0V2);

				if(isnan(normal.x))
				{
					int k = 0;
				}

				normal.Normalize();
				if (isnan(normal.x))
				{
					int k = 0;
				}

				normals.push_back(normal);
			}

			return true;
		}
#pragma warning(pop)
	}
}