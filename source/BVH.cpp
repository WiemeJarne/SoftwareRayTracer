#include "BVH.h"

BVH::BVH(int amountOfTriangles, std::vector<Vector3>& centers, std::vector<Vector3>& positions) :
	m_ptrRootNode{ new BVHNode[amountOfTriangles] }
{
	for (int index{}; index < amountOfTriangles; ++index)
	{
		centers[index] = (positions[index] + positions[index + 2] + positions[index + 3]) * 0.3333f;
	}
}