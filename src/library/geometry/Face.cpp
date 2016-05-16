#include "Face.h"

namespace PR
{
	Face::Face()
	{
		for (int i = 0; i < 3; i++)
		{
			V[i] = PM::pm_Set(0, 0, 0, 1);
			N[i] = PM::pm_Zero();
			UV[i] = PM::pm_Zero();
		}
	}

	Face::~Face()
	{
	}
}