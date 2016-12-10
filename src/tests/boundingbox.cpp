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

PR_TEST("Intersects Left");
{
	Ray ray(0,0, PM::pm_Set(-2, 0, 0), PM::pm_Set(1, 0, 0));
	BoundingBox box(2, 2, 2);

	PM::vec3 collisionPoint;
	float t;
	BoundingBox::FaceSide side;
	PR_CHECK_TRUE(box.intersects(ray, collisionPoint, t, side));
	PR_CHECK_EQ_3(collisionPoint, PM::pm_Set(-1, 0, 0));
	PR_CHECK_EQ(side, BoundingBox::FS_Left);
}

PR_TEST("Intersects Right Inside");
{
	Ray ray(0,0, PM::pm_Set(0, 0, 0), PM::pm_Set(1, 0, 0));
	BoundingBox box(2, 2, 2);

	PM::vec3 collisionPoint;
	float t;
	BoundingBox::FaceSide side;
	PR_CHECK_TRUE(box.intersects(ray, collisionPoint, t, side));
	PR_CHECK_EQ_3(collisionPoint, PM::pm_Set(1, 0, 0));
	PR_CHECK_EQ(side, BoundingBox::FS_Right);
}

PR_TEST("Intersects Right");
{
	Ray ray(0,0, PM::pm_Set(2, 0, 0), PM::pm_Set(-1, 0, 0));
	BoundingBox box(2, 2, 2);

	PM::vec3 collisionPoint;
	float t;
	BoundingBox::FaceSide side;
	PR_CHECK_TRUE(box.intersects(ray, collisionPoint, t, side));
	PR_CHECK_EQ_3(collisionPoint, PM::pm_Set(1, 0, 0));
	PR_CHECK_EQ(side, BoundingBox::FS_Right);
}

PR_TEST("Intersects Front");
{
	Ray ray(0,0, PM::pm_Set(0, 0, -2), PM::pm_Set(0, 0, 1));
	BoundingBox box(2, 2, 2);

	PM::vec3 collisionPoint;
	float t;
	BoundingBox::FaceSide side;
	PR_CHECK_TRUE(box.intersects(ray, collisionPoint, t, side));
	PR_CHECK_EQ_3(collisionPoint, PM::pm_Set(0, 0, -1));
	PR_CHECK_EQ(side, BoundingBox::FS_Front);
}

PR_TEST("Intersects Back Inside");
{
	Ray ray(0,0, PM::pm_Set(0, 0, 0), PM::pm_Set(0, 0, 1));
	BoundingBox box(2, 2, 2);

	PM::vec3 collisionPoint;
	float t;
	BoundingBox::FaceSide side;
	PR_CHECK_TRUE(box.intersects(ray, collisionPoint, t, side));
	PR_CHECK_EQ_3(collisionPoint, PM::pm_Set(0, 0, 1));
	PR_CHECK_EQ(side, BoundingBox::FS_Back);
}

PR_TEST("Intersects Back");
{
	Ray ray(0,0, PM::pm_Set(0, 0, 2), PM::pm_Set(0, 0, -1));
	BoundingBox box(2, 2, 2);

	PM::vec3 collisionPoint;
	float t;
	BoundingBox::FaceSide side;
	PR_CHECK_TRUE(box.intersects(ray, collisionPoint, t, side));
	PR_CHECK_EQ_3(collisionPoint, PM::pm_Set(0, 0, 1));
	PR_CHECK_EQ(side, BoundingBox::FS_Back);
}

PR_TEST("Intersects Bottom");
{
	Ray ray(0,0, PM::pm_Set(0, -2, 0), PM::pm_Set(0, 1, 0));
	BoundingBox box(2, 2, 2);

	PM::vec3 collisionPoint;
	float t;
	BoundingBox::FaceSide side;
	PR_CHECK_TRUE(box.intersects(ray, collisionPoint, t, side));
	PR_CHECK_EQ_3(collisionPoint, PM::pm_Set(0, -1, 0));
	PR_CHECK_EQ(side, BoundingBox::FS_Bottom);
}

PR_TEST("Intersects Top Inside");
{
	Ray ray(0,0, PM::pm_Set(0, 0, 0), PM::pm_Set(0, 1, 0));
	BoundingBox box(2, 2, 2);

	PM::vec3 collisionPoint;
	float t;
	BoundingBox::FaceSide side;
	PR_CHECK_TRUE(box.intersects(ray, collisionPoint, t, side));
	PR_CHECK_EQ_3(collisionPoint, PM::pm_Set(0, 1, 0));
	PR_CHECK_EQ(side, BoundingBox::FS_Top);
}

PR_TEST("Intersects Top");
{
	Ray ray(0,0, PM::pm_Set(0, 2, 0), PM::pm_Set(0, -1, 0));
	BoundingBox box(2, 2, 2);

	PM::vec3 collisionPoint;
	float t;
	BoundingBox::FaceSide side;
	PR_CHECK_TRUE(box.intersects(ray, collisionPoint, t, side));
	PR_CHECK_EQ_3(collisionPoint, PM::pm_Set(0, 1, 0));
	PR_CHECK_EQ(side, BoundingBox::FS_Top);
}

PR_TEST("Intersects Complex");
{
	PM::vec3 point = PM::pm_Set(0, 1, 0);
	Ray ray(0,0, PM::pm_Set(1, 2, 0), PM::pm_Normalize3D(PM::pm_Set(-1, -1, 0)));
	BoundingBox box(2, 2, 2);

	PM::vec3 collisionPoint;
	float t;
	BoundingBox::FaceSide side;
	PR_CHECK_TRUE(box.intersects(ray, collisionPoint, t, side));
	PR_CHECK_NEARLY_EQ_3(collisionPoint, point);
	PR_CHECK_EQ(side, BoundingBox::FS_Top);
}

PR_TEST("Face Front");
{
	BoundingBox box(1, 2, 3);
	Plane plane = box.getFace(BoundingBox::FS_Front);
	PR_CHECK_EQ_3(plane.normal(), PM::pm_Set(0, 0, 1));
	PR_CHECK_NEARLY_EQ(PM::pm_GetZ(plane.position()), -3 / 2.0f);
	PR_CHECK_EQ(PM::pm_Magnitude3D(plane.xAxis()), 1);
	PR_CHECK_EQ(PM::pm_Magnitude3D(plane.yAxis()), 2);
}

PR_TEST("Face Back");
{
	BoundingBox box(1, 2, 3);
	Plane plane = box.getFace(BoundingBox::FS_Back);
	PR_CHECK_EQ_3(plane.normal(), PM::pm_Set(0, 0, -1));
	PR_CHECK_NEARLY_EQ(PM::pm_GetZ(plane.position()), 3 / 2.0f);
	PR_CHECK_EQ(PM::pm_Magnitude3D(plane.xAxis()), 1);
	PR_CHECK_EQ(PM::pm_Magnitude3D(plane.yAxis()), 2);
}

PR_TEST("Face Left");
{
	BoundingBox box(1, 2, 3);
	Plane plane = box.getFace(BoundingBox::FS_Left);
	PR_CHECK_EQ_3(plane.normal(), PM::pm_Set(1, 0, 0));
	PR_CHECK_NEARLY_EQ(PM::pm_GetX(plane.position()), -1 / 2.0f);
	PR_CHECK_EQ(PM::pm_Magnitude3D(plane.xAxis()), 3);
	PR_CHECK_EQ(PM::pm_Magnitude3D(plane.yAxis()), 2);
}

PR_TEST("Face Right");
{
	BoundingBox box(1, 2, 3);
	Plane plane = box.getFace(BoundingBox::FS_Right);
	PR_CHECK_EQ_3(plane.normal(), PM::pm_Set(-1, 0, 0));
	PR_CHECK_NEARLY_EQ(PM::pm_GetX(plane.position()), 1 / 2.0f);
	PR_CHECK_EQ(PM::pm_Magnitude3D(plane.xAxis()), 3);
	PR_CHECK_EQ(PM::pm_Magnitude3D(plane.yAxis()), 2);
}

PR_TEST("Face Top");
{
	BoundingBox box(1, 2, 3);
	Plane plane = box.getFace(BoundingBox::FS_Top);
	PR_CHECK_EQ_3(plane.normal(), PM::pm_Set(0, -1, 0));
	PR_CHECK_NEARLY_EQ(PM::pm_GetY(plane.position()), 1);
	PR_CHECK_EQ(PM::pm_Magnitude3D(plane.xAxis()), 1);
	PR_CHECK_EQ(PM::pm_Magnitude3D(plane.yAxis()), 3);
}

PR_TEST("Face Bottom");
{
	BoundingBox box(1, 2, 3);
	Plane plane = box.getFace(BoundingBox::FS_Bottom);
	PR_CHECK_EQ_3(plane.normal(), PM::pm_Set(0, 1, 0));
	PR_CHECK_NEARLY_EQ(PM::pm_GetY(plane.position()), -1);
	PR_CHECK_EQ(PM::pm_Magnitude3D(plane.xAxis()), 1);
	PR_CHECK_EQ(PM::pm_Magnitude3D(plane.yAxis()), 3);
}

PR_END_TESTCASE()

// MAIN
PRT_BEGIN_MAIN
PRT_TESTCASE(BoundingBox);
PRT_END_MAIN