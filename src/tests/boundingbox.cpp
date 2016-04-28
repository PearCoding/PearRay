#include "geometry/BoundingBox.h"
#include "geometry/Plane.h"
#include "ray/Ray.h"

#include "Test.h"

using namespace PR;

PR_BEGIN_TESTCASE(BoundingBox)
PR_TEST("Size");
{
	BoundingBox box(10, 10, 10);
	PR_CHECK_EQ(box.width(), 10);
	PR_CHECK_EQ(box.height(), 10);
	PR_CHECK_EQ(box.depth(), 10);
	PR_CHECK_EQ(box.volume(), 1000);
}

PR_TEST("Bounds");
{
	BoundingBox box(2, 2, 2);
	PR_CHECK_EQ_3(box.lowerBound(), PM::pm_Set(-1, -1, -1));
	PR_CHECK_EQ_3(box.upperBound(), PM::pm_Set(1, 1, 1));
	PR_CHECK_EQ_3(box.center(), PM::pm_Set(0, 0, 0));
}

PR_TEST("Face Front");
{
	BoundingBox box(1, 2, 3);
	Plane plane = box.getFace(BoundingBox::FS_Front);
	PR_CHECK_EQ_3(plane.normal(), PM::pm_Set(0, 0, -1));
	PR_CHECK_EQ(plane.width(), 1);
	PR_CHECK_EQ(plane.height(), 2);
}

PR_TEST("Face Back");
{
	BoundingBox box(1, 2, 3);
	Plane plane = box.getFace(BoundingBox::FS_Back);
	PR_CHECK_EQ_3(plane.normal(), PM::pm_Set(0, 0, 1));
	PR_CHECK_EQ(plane.width(), 1);
	PR_CHECK_EQ(plane.height(), 2);
}

PR_TEST("Face Left");
{
	BoundingBox box(1, 2, 3);
	Plane plane = box.getFace(BoundingBox::FS_Left);
	PR_CHECK_EQ_3(plane.normal(), PM::pm_Set(-1, 0, 0));
	PR_CHECK_EQ(plane.width(), 3);
	PR_CHECK_EQ(plane.height(), 2);
}

PR_TEST("Face Right");
{
	BoundingBox box(1, 2, 3);
	Plane plane = box.getFace(BoundingBox::FS_Right);
	PR_CHECK_EQ_3(plane.normal(), PM::pm_Set(1, 0, 0));
	PR_CHECK_EQ(plane.width(), 3);
	PR_CHECK_EQ(plane.height(), 2);
}

PR_TEST("Face Top");
{
	BoundingBox box(1, 2, 3);
	Plane plane = box.getFace(BoundingBox::FS_Top);
	PR_CHECK_EQ_3(plane.normal(), PM::pm_Set(0, 1, 0));
	PR_CHECK_EQ(plane.width(), 1);
	PR_CHECK_EQ(plane.height(), 3);
}

PR_TEST("Face Bottom");
{
	BoundingBox box(1, 2, 3);
	Plane plane = box.getFace(BoundingBox::FS_Bottom);
	PR_CHECK_EQ_3(plane.normal(), PM::pm_Set(0, -1, 0));
	PR_CHECK_EQ(plane.width(), 1);
	PR_CHECK_EQ(plane.height(), 3);
}

PR_END_TESTCASE()

// MAIN
PRT_BEGIN_MAIN
PRT_TESTCASE(BoundingBox);
PRT_END_MAIN