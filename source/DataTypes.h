#pragma once
#include <cassert>
#include <iostream>
#include "Math.h"
#include "vector"
#include <string>

namespace dae
{
	enum class MaterialType
	{
		solidColor,
		lambert,
		lambertPhong,
		cookTorrence
	};

	struct BVHNode
	{
		Vector3 AABBMin;
		Vector3 AABBMax;
		int leftChildIndex; //rightChildIndex = leftChildIndex + 1
		int firstMeshIndex;
		int amountOfMeshes;
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

		void Grow(Vector3 position)
		{
			min = Vector3::Min(min, position);
			max = Vector3::Max(max, position);
		}

		float Area()
		{
			Vector3 extent{ max - min };
			return extent.x * extent.y + extent.y * extent.z + extent.z * extent.x;
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
		std::vector<Vector3> centroids{};
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

		std::vector<BVHNode> bvhNodes;
		int rootNodeIndex{ 0 }, amountOfUsedNodes{ 1 };

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
			int v0Index{ -1 };
			int v1Index{ -1 };
			int v2Index{ -1 };

			int amountOfPositions{ static_cast<int>(positions.size()) };
			for (int index{}; index < amountOfPositions; ++index)
			{
				if (positions[index] == triangle.v0)
				{
					v0Index = index;
				}

				if (positions[index] == triangle.v1)
				{
					v1Index = index;
				}

				if (positions[index] == triangle.v2)
				{
					v2Index = index;
				}
			}

			if (v0Index != -1)
			{
				indices.push_back(v0Index);
			}
			else
			{
				int newIndex{ static_cast<int>(indices.size()) };
				indices.push_back(newIndex);
				positions.push_back(triangle.v0);
			}

			if (v1Index != -1)
			{
				indices.push_back(v1Index);
			}
			else
			{
				int newIndex{ static_cast<int>(indices.size()) };
				indices.push_back(newIndex);
				positions.push_back(triangle.v1);
			}

			if (v2Index != -1)
			{
				indices.push_back(v2Index);
			}
			else
			{
				int newIndex{ static_cast<int>(indices.size()) };
				indices.push_back(newIndex);
				positions.push_back(triangle.v2);
			}

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
				transformedNormals.emplace_back(finalTransform.TransformVector(normals[index]).Normalized());
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

		//bvh functions
		//source for bvh: https://jacco.ompf2.com/2022/04/13/how-to-build-a-bvh-part-1-basics/
		void BuildBVH()
		{
			int amountOfTriangles{ static_cast<int>(indices.size()) / 3 };

			bvhNodes.reserve(amountOfTriangles * 2 - 1);
			int maxAmountOfBVHNodes{ static_cast<int>(bvhNodes.capacity()) };
			for (int index{}; index < maxAmountOfBVHNodes; ++index)
			{
				bvhNodes.push_back(BVHNode{});
			}

			//assign all triangles to root node
			bvhNodes[rootNodeIndex].leftChildIndex = 0;
			bvhNodes[rootNodeIndex].amountOfMeshes = amountOfTriangles;

			UpdateNodeBounds(rootNodeIndex);
			//subdivide recursively
			Subdivide(rootNodeIndex);
		}

		void UpdateNodeBounds(int nodeIndex)
		{
			BVHNode& node{ bvhNodes[nodeIndex] };
			node.AABBMin = { INFINITY, INFINITY, INFINITY };
			node.AABBMax = { -INFINITY, -INFINITY, -INFINITY };

			int start{ node.leftChildIndex * 3 };
			int end{ start + node.amountOfMeshes * 3 };
			
			for (int index{ start }; index < end; ++index)
			{
				node.AABBMin = Vector3::Min(node.AABBMin, transformedPositions[indices[index]]);
				node.AABBMax = Vector3::Max(node.AABBMax, transformedPositions[indices[index]]);
			}
		}

		void Subdivide(int nodeIndex)
		{
			BVHNode& node{ bvhNodes[nodeIndex] };

			int bestAxis{ -1 };
			float bestPos{};
			float bestCost{ INFINITY };

			for (int axis{}; axis < 3; ++axis)
			{
				for (int index{}; index < node.amountOfMeshes; ++index)
				{
					const int triangleIndex{ node.leftChildIndex + index };
					const Vector3 centroid
					{
						(  transformedPositions[indices[triangleIndex * 3]]
						 + transformedPositions[indices[triangleIndex * 3 + 1]]
						 + transformedPositions[indices[triangleIndex * 3 + 2]]) / 3.f
					};

					const float candidatePos{ centroid[axis] };
					const float cost{ EvaluateSAH(node, axis, candidatePos) };

					if (cost < bestCost)
					{
						bestAxis = axis;
						bestPos = candidatePos;
						bestCost = cost;
					}
				}
			}

			const Vector3 extentParent{ node.AABBMax - node.AABBMin };
			const float areaParent{ extentParent.x * extentParent.y + extentParent.y * extentParent.z + extentParent.z * extentParent.x };
			const float costParent{ node.amountOfMeshes * areaParent };

			if (bestCost >= costParent) return;

			int left{ node.leftChildIndex };
			const int right{ left + node.amountOfMeshes - 1 };

			SortPrimitives(left, right, bestAxis, bestPos);

			int leftCount{ left - node.leftChildIndex };
			if (leftCount == 0 || leftCount == node.amountOfMeshes) return;

			//create child nodes
			int leftChildIndex{ amountOfUsedNodes };
			amountOfUsedNodes += 2;
			bvhNodes[leftChildIndex].leftChildIndex = node.leftChildIndex;
			bvhNodes[leftChildIndex].amountOfMeshes = leftCount;
			bvhNodes[leftChildIndex + 1].leftChildIndex = left; //leftChildIndex + 1 == rightChildIndex (rightChildIndex is not saved in the node)
			bvhNodes[leftChildIndex + 1].amountOfMeshes = node.amountOfMeshes - leftCount;

			node.amountOfMeshes = 0;
			node.leftChildIndex = leftChildIndex;

			UpdateNodeBounds(leftChildIndex);
			UpdateNodeBounds(leftChildIndex + 1);

			//recurse
			Subdivide(leftChildIndex);
			Subdivide(leftChildIndex + 1);
		}

		float EvaluateSAH(const BVHNode& node, int axis, float position)
		{
			AABB leftBox{};
			AABB rightBox{};
			int leftCount{};
			int rightCount{};

			for (int index{}; index < node.amountOfMeshes; ++index)
			{
				const int triangleIndex{ node.leftChildIndex + index };
				const Vector3 centroid
				{
					(  transformedPositions[indices[triangleIndex * 3]]
					 + transformedPositions[indices[triangleIndex * 3 + 1]]
					 + transformedPositions[indices[triangleIndex * 3 + 2]] ) / 3.f
				};

				if (centroid[axis] < position)
				{
					++leftCount;

					leftBox.Grow(transformedPositions[indices[triangleIndex * 3]]);
					leftBox.Grow(transformedPositions[indices[triangleIndex * 3 + 1]]);
					leftBox.Grow(transformedPositions[indices[triangleIndex * 3 + 2]]);
				}
				else
				{
					++rightCount;

					rightBox.Grow(transformedPositions[indices[triangleIndex * 3]]);
					rightBox.Grow(transformedPositions[indices[triangleIndex * 3 + 1]]);
					rightBox.Grow(transformedPositions[indices[triangleIndex * 3 + 2]]);
				}
			}
			float cost{ leftCount * leftBox.Area() + rightCount * rightBox.Area() };
			return cost > 0 ? cost : INFINITY;
		}

		void SortPrimitives(int& left, int right, int axis, float splitPosition)
		{
			while (left <= right)
			{
				const Vector3 centroid
				{
					(  transformedPositions[indices[left * 3]]
					 + transformedPositions[indices[left * 3 + 1]]
					 + transformedPositions[indices[left * 3 + 2]] ) / 3.f
				};

				if (centroid[axis] < splitPosition) ++left;
				else
				{
					std::swap(normals[left], normals[right]);
					std::swap(transformedNormals[left], transformedNormals[right]);
					std::swap(indices[left * 3], indices[right * 3]);
					std::swap(indices[left * 3 + 1], indices[right * 3 + 1]);
					std::swap(indices[left * 3 + 2], indices[right * 3 + 2]);
					--right;
				}
			}
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
}