#pragma once
#include <cassert>

#include "Math.h"
#include "vector"

namespace dae
{
	enum class MaterialType
	{
		solidColor,
		lambert,
		lambertPhong,
		cookTorrence
	};

#pragma region GEOMETRY
	struct Sphere
	{
		Vector3 origin{};
		float radius{};

		unsigned char materialIndex{ 0 };
		MaterialType materialType{};
	};

	struct Plane
	{
		Vector3 origin{};
		Vector3 normal{};

		unsigned char materialIndex{ 0 };
		MaterialType materialType{};
	};

	enum class TriangleCullMode
	{
		FrontFaceCulling,
		BackFaceCulling,
		NoCulling
	};

	struct Triangle
	{
		Triangle() = default;
		Triangle(const Vector3& _v0, const Vector3& _v1, const Vector3& _v2, const Vector3& _normal):
			v0{_v0}, v1{_v1}, v2{_v2}, normal{_normal.Normalized()}{}

		Triangle(const Vector3& _v0, const Vector3& _v1, const Vector3& _v2) :
			v0{ _v0 }, v1{ _v1 }, v2{ _v2 }
		{
			const Vector3 edgeV0V1 = v1 - v0;
			const Vector3 edgeV0V2 = v2 - v0;
			normal = Vector3::Cross(edgeV0V1, edgeV0V2).Normalized();
		}

		Vector3 v0{};
		Vector3 v1{};
		Vector3 v2{};

		Vector3 normal{};

		TriangleCullMode cullMode{};
		unsigned char materialIndex{};
		MaterialType materialType{};
	};

	struct AABB
	{
		Vector3 min{ INFINITY, INFINITY, INFINITY };
		Vector3 max{ -INFINITY, -INFINITY, -INFINITY };

		void grow(Vector3 position)
		{
			min = Vector3::Min(min, position);
			max = Vector3::Max(max, position);
		}
	};

	struct TriangleMesh
	{
		TriangleMesh() = default;
		TriangleMesh(const std::vector<Vector3>& _positions, const std::vector<int>& _indices, TriangleCullMode _cullMode):
		positions(_positions), indices(_indices), cullMode(_cullMode)
		{
			//Calculate Normals
			CalculateNormals();

			//Update Transforms
			UpdateTransforms();
		}

		TriangleMesh(const std::vector<Vector3>& _positions, const std::vector<int>& _indices, const std::vector<Vector3>& _normals, TriangleCullMode _cullMode) :
			positions(_positions), indices(_indices), normals(_normals), cullMode(_cullMode)
		{
			UpdateTransforms();
		}

		std::vector<Vector3> positions{};
		std::vector<Vector3> normals{};
		std::vector<int> indices{};
		unsigned char materialIndex{};
		MaterialType materialType{};

		TriangleCullMode cullMode{TriangleCullMode::BackFaceCulling};

		Matrix rotationTransform{};
		Matrix translationTransform{};
		Matrix scaleTransform{};

		Vector3 minAABB;
		Vector3 maxAABB;

		Vector3 transformedMinAABB;
		Vector3 transformedMaxAABB;

		std::vector<Vector3> transformedPositions{};
		std::vector<Vector3> transformedNormals{};

		void Translate(const Vector3& translation)
		{
			translationTransform = Matrix::CreateTranslation(translation);
		}

		void RotateY(float yaw)
		{
			rotationTransform = Matrix::CreateRotationY(yaw);
		}

		void Scale(const Vector3& scale)
		{
			scaleTransform = Matrix::CreateScale(scale);
		}

		void AppendTriangle(const Triangle& triangle, bool ignoreTransformUpdate = false)
		{
			int startIndex = static_cast<int>(positions.size());

			positions.push_back(triangle.v0);
			positions.push_back(triangle.v1);
			positions.push_back(triangle.v2);

			indices.push_back(startIndex);
			indices.push_back(++startIndex);
			indices.push_back(++startIndex);

			normals.push_back(triangle.normal);

			//Not ideal, but making sure all vertices are updated
			if(!ignoreTransformUpdate)
				UpdateTransforms();
		}

		void CalculateNormals()
		{
			const size_t amountOfIndices{ indices.size()};
			for(size_t index{}; index < amountOfIndices; ++index)
			{
				Vector3 v0{ positions[indices[index]] };
				++index;
				Vector3 v1{ positions[indices[index]] };
				++index;
				Vector3 v2{ positions[indices[index]] };

				Vector3 edge0{ v1 - v0 };
				Vector3 edge1{ v2 - v0 };

				normals.emplace_back(Vector3::Cross(edge0, edge1).Normalized());
			}
		}

		void UpdateTransforms()
		{
			//Calculate Final Transform 
			//const auto finalTransform = ...
			const auto finalTransform = scaleTransform * rotationTransform * translationTransform;
			//Transform Positions (positions > transformedPositions)
			//...
			transformedPositions.resize(0);
			const size_t amountOfPositions{ positions.size() };
			for(size_t index{}; index < amountOfPositions; ++index)
			{
				transformedPositions.emplace_back(finalTransform.TransformPoint(positions[index]));

				UpdateTransformedAABB(finalTransform);
			}
			//Transform Normals (normals > transformedNormals)
			//...
			transformedNormals.resize(0);
			const size_t amountOfNormals{ normals.size() };
			for(size_t index{}; index < amountOfNormals; ++index)
			{
				transformedNormals.emplace_back(finalTransform.TransformVector(normals[index]));
			}
		}

		void UpdateAABB()
		{
			if (!positions.empty())
			{
				minAABB = positions[0];
				maxAABB = positions[0];

				for (auto& position : positions)
				{
					minAABB = Vector3::Min(position, minAABB);
					maxAABB = Vector3::Max(position, maxAABB);
				}
			}
		}

		void UpdateTransformedAABB(const Matrix& finalTransform)
		{
			//AABB Update: be careful -> transform the 8 vertices fo the AABB and calculate new min and max
			Vector3 tMinAABB{ finalTransform.TransformPoint(minAABB) };
			Vector3 tMaxAABB{ tMinAABB };
			
			//xMax, yMin, zMin
			Vector3 tAABB{ finalTransform.TransformPoint(maxAABB.x, minAABB.y, minAABB.z) };
			tMinAABB = Vector3::Min(tAABB, tMinAABB);
			tMaxAABB = Vector3::Max(tAABB, tMaxAABB);

			//xMax, yMin, zMax
			tAABB = finalTransform.TransformPoint(maxAABB.x, minAABB.y, maxAABB.z);
			tMinAABB = Vector3::Min(tAABB, tMinAABB);
			tMaxAABB = Vector3::Max(tAABB, tMaxAABB);

			//xMin, yMin, zMax
			tAABB = finalTransform.TransformPoint(minAABB.x, minAABB.y, maxAABB.z);
			tMinAABB = Vector3::Min(tAABB, tMinAABB);
			tMaxAABB = Vector3::Max(tAABB, tMaxAABB);

			//xMin, yMax, zMin
			tAABB = finalTransform.TransformPoint(minAABB.x, maxAABB.y, minAABB.z);
			tMinAABB = Vector3::Min(tAABB, tMinAABB);
			tMaxAABB = Vector3::Max(tAABB, tMaxAABB);

			//xMax, yMax, zMin
			tAABB = finalTransform.TransformPoint(maxAABB.x, maxAABB.y, minAABB.z);
			tMinAABB = Vector3::Min(tAABB, tMinAABB);
			tMaxAABB = Vector3::Max(tAABB, tMaxAABB);

			//xMax, yMax, zMax
			tAABB = finalTransform.TransformPoint(maxAABB);
			tMinAABB = Vector3::Min(tAABB, tMinAABB);
			tMaxAABB = Vector3::Max(tAABB, tMaxAABB);

			//xMin, yMax, zax
			tAABB = finalTransform.TransformPoint(minAABB.x, maxAABB.y, maxAABB.z);
			tMinAABB = Vector3::Min(tAABB, tMinAABB);
			tMaxAABB = Vector3::Max(tAABB, tMaxAABB);

			transformedMinAABB = tMinAABB;
			transformedMaxAABB = tMaxAABB;
		}
	};
#pragma endregion
#pragma region LIGHT
	enum class LightType
	{
		Point,
		Directional
	};

	struct Light
	{
		Vector3 origin{};
		Vector3 direction{};
		ColorRGB color{};
		float intensity{};

		LightType type{};
	};
#pragma endregion
#pragma region MISC
	struct Ray
	{
		Vector3 origin{};
		Vector3 direction{};

		float min{ 0.0001f };
		float max{ FLT_MAX };
	};

	struct HitRecord
	{
		Vector3 origin{};
		Vector3 normal{};
		float t = FLT_MAX;

		bool didHit{ false };
		unsigned char materialIndex{ 0 };
		MaterialType materialType;
	};
#pragma endregion
#pragma region BVH
	//source for bvh: https://jacco.ompf2.com/2022/04/13/how-to-build-a-bvh-part-1-basics/
	struct BVHNode
	{
		float AABBMin, AABBMax;
		Uint32 leftChild, rightChild;
		Uint32 firstMesh, amountOfMeshes;
	};

	struct BHV
	{
		BVHNode bvhNode[0]{};
		Uint32 rootNodeIndex = 0, nodesUsed = 1;

		void BuildBVH(BVHNode node, int amountOfTriangle)
		{
			bvhNode = node;
			BVHNode& root = bvhNode[rootNodeIndex];

		}
	};

	
#pragma endregion
}