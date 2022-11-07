//#include <cassert>

#include "vector"
#include "Math.h"

class BVH
{
public:
	BVH(int amountOfTriangles, std::vector<Vector3>& centers, std::vector<Vector3>& positions);

private:
	struct BVHNode
	{
		float AABBMin, AABBMax;
		uint32_t leftChild, rightChild;
		uint32_t firstMesh, amountOfMeshes;
	};

	BVHNode* m_ptrRootNode;
};