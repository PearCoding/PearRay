#include "Face.h"

namespace PR
{
	Face::Face() :
		V1(PM::pm_Set(0, 0, 0, 1)), V2(PM::pm_Set(0, 0, 0, 1)), V3(PM::pm_Set(0, 0, 0, 1)),
		N1(PM::pm_Set(0, 0, 0, 1)), N2(PM::pm_Set(0, 0, 0, 1)), N3(PM::pm_Set(0, 0, 0, 1)),
		UV1(PM::pm_Set(0, 0)), UV2(PM::pm_Set(0, 0)), UV3(PM::pm_Set(0, 0))
	{
	}

	Face::~Face()
	{
	}
}