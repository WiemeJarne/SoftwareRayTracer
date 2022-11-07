#include <cassert>

#include "Math.h"
#include "vector"

class BVH
{
public:
	BVH(int amountOfTriangles);

private:
	struct Tri { float3 vertex0, vertex1, vertex2; float3 centroid; };
	__declspec(align(32)) struct BVHNode
	{
		float3 aabbMin, aabbMax;
		uint leftFirst, triCount;
		bool isLeaf() { return triCount > 0; }
	};

	struct BVHNode
	{
		float AABBMin, AABBMax;
		uint32_t leftChild, rightChild;
		uint32_t firstMesh, amountOfMeshes;
	};

	BVHNode* m_ptrRootNode;
};