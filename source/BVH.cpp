#include "BVH.h"

BVH::BVH(int amountOfTriangles) :
	m_ptrRootNode{ new BVHNode[amountOfTriangles] }
{
	for (int index{}; index < amountOfTriangles; ++index)
	{

	}
}



