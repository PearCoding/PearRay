#include "geometry/BoundingBox.h"
#include "geometry/Plane.h"
#include "ray/Ray.h"

#include "Test.h"

using namespace PR;

PR_BEGIN_TESTCASE(BoundingBox)
PR_TEST("Validity")
{
	BoundingBox box(10, 10, 10);
	PR_CHECK_TRUE(box.isValid());
}

PR_TEST("Invalidity")
{
	BoundingBox box;
	PR_CHECK_FALSE(box.isValid());
}

PR_TEST("Size")
{
	BoundingBox box(10, 10, 10);
	PR_CHECK_EQ(box.width(), 10);
	PR_CHECK_EQ(box.height(), 10);
	PR_CHECK_EQ(box.depth(), 10);
	PR_CHECK_EQ(box.volume(), 1000);
}

PR_TEST("Bounds")
{
	BoundingBox box(2, 2, 2);
	PR_CHECK_EQ(box.lowerBound(), Eigen::Vector3f(-1, -1, -1));
	PR_CHECK_EQ(box.upperBound(), Eigen::Vector3f(1, 1, 1));
	PR_CHECK_EQ(box.center(), Eigen::Vector3f(0, 0, 0));
}

PR_TEST("Clip Inside")
{
	BoundingBox box(2, 2, 2);
	BoundingBox parent(3, 3, 3);

	box.clipBy(parent);
	PR_CHECK_EQ(box.width(), 2);
	PR_CHECK_EQ(box.height(), 2);
	PR_CHECK_EQ(box.depth(), 2);
	PR_CHECK_EQ(box.center(), Eigen::Vector3f(0, 0, 0));
}

PR_TEST("Clip Outside")
{
	BoundingBox box(2, 2, 2);
	BoundingBox parent(1, 1, 1);

	box.clipBy(parent);
	PR_CHECK_EQ(box.width(), 1);
	PR_CHECK_EQ(box.height(), 1);
	PR_CHECK_EQ(box.depth(), 1);
	PR_CHECK_EQ(box.center(), Eigen::Vector3f(0, 0, 0));
}

PR_TEST("Clip Zero")
{
	BoundingBox box(Eigen::Vector3f(0,0,0), Eigen::Vector3f(1,1,1));
	BoundingBox parent(Eigen::Vector3f(-1,-1,-1), Eigen::Vector3f(0,0,0));

	box.clipBy(parent);
	PR_CHECK_EQ(box.width(), 0);
	PR_CHECK_EQ(box.height(), 0);
	PR_CHECK_EQ(box.depth(), 0);
	PR_CHECK_EQ(box.lowerBound(), Eigen::Vector3f(0,0,0));
	PR_CHECK_EQ(box.upperBound(), Eigen::Vector3f(0,0,0));
}

PR_TEST("Clip Left Half")
{
	BoundingBox box(Eigen::Vector3f(0,1,2), Eigen::Vector3f(2,3,4));
	BoundingBox parent(Eigen::Vector3f(-1,0,1), Eigen::Vector3f(1,2,3));

	box.clipBy(parent);
	PR_CHECK_EQ(box.width(), 1);
	PR_CHECK_EQ(box.height(), 1);
	PR_CHECK_EQ(box.depth(), 1);
	PR_CHECK_EQ(box.lowerBound(), Eigen::Vector3f(0,1,2));
	PR_CHECK_EQ(box.upperBound(), Eigen::Vector3f(1,2,3));
}

PR_TEST("Clip Right Half")
{
	BoundingBox box(Eigen::Vector3f(-2,-3,-4), Eigen::Vector3f(1,0,-1));
	BoundingBox parent(Eigen::Vector3f(-1,-2,-3), Eigen::Vector3f(1,2,3));

	box.clipBy(parent);
	PR_CHECK_EQ(box.width(), 2);
	PR_CHECK_EQ(box.height(), 2);
	PR_CHECK_EQ(box.depth(), 2);
	PR_CHECK_EQ(box.lowerBound(), Eigen::Vector3f(-1,-2,-3));
	PR_CHECK_EQ(box.upperBound(), Eigen::Vector3f(1,0,-1));
}

PR_TEST("Inflate")
{
	BoundingBox box(0, 0, 0);
	box.inflate(0.1);
	PR_CHECK_EQ(box.lowerBound(), Eigen::Vector3f(-0.1, -0.1, -0.1));
	PR_CHECK_EQ(box.upperBound(), Eigen::Vector3f(0.1, 0.1, 0.1));
	PR_CHECK_EQ(box.center(), Eigen::Vector3f(0, 0, 0));
}

PR_TEST("Inflate Max")
{
	BoundingBox box(0, 0, 0);
	box.inflate(0.1, true);
	PR_CHECK_EQ(box.lowerBound(), Eigen::Vector3f(0, 0, 0));
	PR_CHECK_EQ(box.upperBound(), Eigen::Vector3f(0.1, 0.1, 0.1));
	PR_CHECK_EQ(box.center(), Eigen::Vector3f(0.05, 0.05, 0.05));
}

PR_TEST("Intersects Left")
{
	Ray ray(0,0, Eigen::Vector3f(-2, 0, 0), Eigen::Vector3f(1, 0, 0));
	BoundingBox box(2, 2, 2);

	BoundingBox::Intersection s = box.intersects(ray);
	PR_CHECK_TRUE(s.Successful);
	PR_CHECK_EQ(s.Position, Eigen::Vector3f(-1, 0, 0));
	PR_CHECK_EQ(box.getIntersectionSide(s), BoundingBox::FS_Left);
}

PR_TEST("Intersects Right Inside")
{
	Ray ray(0,0, Eigen::Vector3f(0, 0, 0), Eigen::Vector3f(1, 0, 0));
	BoundingBox box(2, 2, 2);

	 	BoundingBox::Intersection s = box.intersects(ray);
	PR_CHECK_TRUE(s.Successful);
	PR_CHECK_EQ(s.Position, Eigen::Vector3f(1, 0, 0));
	PR_CHECK_EQ(box.getIntersectionSide(s), BoundingBox::FS_Right);
}

PR_TEST("Intersects Right")
{
	Ray ray(0,0, Eigen::Vector3f(2, 0, 0), Eigen::Vector3f(-1, 0, 0));
	BoundingBox box(2, 2, 2);

	 	BoundingBox::Intersection s = box.intersects(ray);
	PR_CHECK_TRUE(s.Successful);
	PR_CHECK_EQ(s.Position, Eigen::Vector3f(1, 0, 0));
	PR_CHECK_EQ(box.getIntersectionSide(s), BoundingBox::FS_Right);
}

PR_TEST("Intersects Front")
{
	Ray ray(0,0, Eigen::Vector3f(0, 0, -2), Eigen::Vector3f(0, 0, 1));
	BoundingBox box(2, 2, 2);

	 	BoundingBox::Intersection s = box.intersects(ray);
	PR_CHECK_TRUE(s.Successful);
	PR_CHECK_EQ(s.Position, Eigen::Vector3f(0, 0, -1));
	PR_CHECK_EQ(box.getIntersectionSide(s), BoundingBox::FS_Front);
}

PR_TEST("Intersects Back Inside")
{
	Ray ray(0,0, Eigen::Vector3f(0, 0, 0), Eigen::Vector3f(0, 0, 1));
	BoundingBox box(2, 2, 2);

	 	BoundingBox::Intersection s = box.intersects(ray);
	PR_CHECK_TRUE(s.Successful);
	PR_CHECK_EQ(s.Position, Eigen::Vector3f(0, 0, 1));
	PR_CHECK_EQ(box.getIntersectionSide(s), BoundingBox::FS_Back);
}

PR_TEST("Intersects Back")
{
	Ray ray(0,0, Eigen::Vector3f(0, 0, 2), Eigen::Vector3f(0, 0, -1));
	BoundingBox box(2, 2, 2);

	 	BoundingBox::Intersection s = box.intersects(ray);
	PR_CHECK_TRUE(s.Successful);
	PR_CHECK_EQ(s.Position, Eigen::Vector3f(0, 0, 1));
	PR_CHECK_EQ(box.getIntersectionSide(s), BoundingBox::FS_Back);
}

PR_TEST("Intersects Bottom")
{
	Ray ray(0,0, Eigen::Vector3f(0, -2, 0), Eigen::Vector3f(0, 1, 0));
	BoundingBox box(2, 2, 2);

	 	BoundingBox::Intersection s = box.intersects(ray);
	PR_CHECK_TRUE(s.Successful);
	PR_CHECK_EQ(s.Position, Eigen::Vector3f(0, -1, 0));
	PR_CHECK_EQ(box.getIntersectionSide(s), BoundingBox::FS_Bottom);
}

PR_TEST("Intersects Top Inside")
{
	Ray ray(0,0, Eigen::Vector3f(0, 0, 0), Eigen::Vector3f(0, 1, 0));
	BoundingBox box(2, 2, 2);

	 	BoundingBox::Intersection s = box.intersects(ray);
	PR_CHECK_TRUE(s.Successful);
	PR_CHECK_EQ(s.Position, Eigen::Vector3f(0, 1, 0));
	PR_CHECK_EQ(box.getIntersectionSide(s), BoundingBox::FS_Top);
}

PR_TEST("Intersects Top")
{
	Ray ray(0,0, Eigen::Vector3f(0, 2, 0), Eigen::Vector3f(0, -1, 0));
	BoundingBox box(2, 2, 2);

	 	BoundingBox::Intersection s = box.intersects(ray);
	PR_CHECK_TRUE(s.Successful);
	PR_CHECK_EQ(s.Position, Eigen::Vector3f(0, 1, 0));
	PR_CHECK_EQ(box.getIntersectionSide(s), BoundingBox::FS_Top);
}

PR_TEST("Intersects Complex")
{
	Eigen::Vector3f point = Eigen::Vector3f(0, 1, 0);
	Ray ray(0,0, Eigen::Vector3f(1, 2, 0), Eigen::Vector3f(-1, -1, 0).normalized());
	BoundingBox box(2, 2, 2);

	 	BoundingBox::Intersection s = box.intersects(ray);
	PR_CHECK_TRUE(s.Successful);
	PR_CHECK_NEARLY_EQ(s.Position, point);
	PR_CHECK_EQ(box.getIntersectionSide(s), BoundingBox::FS_Top);
}

PR_TEST("Intersects Range")
{
	Ray ray(0,0, Eigen::Vector3f(-2, 0, 0), Eigen::Vector3f(1, 0, 0));
	BoundingBox box(2, 2, 2);

	BoundingBox::IntersectionRange s = box.intersectsRange(ray);
	PR_CHECK_TRUE(s.Successful);
	PR_CHECK_EQ(s.Entry, 1);
	PR_CHECK_EQ(s.Exit, 3);
}

PR_TEST("Face Front")
{
	BoundingBox box(1, 2, 3);
	Plane plane = box.getFace(BoundingBox::FS_Front);
	PR_CHECK_EQ(plane.normal(), Eigen::Vector3f(0, 0, 1));
	PR_CHECK_NEARLY_EQ(plane.position()(2), -3 / 2.0f);
	PR_CHECK_EQ(plane.xAxis().norm(), 1);
	PR_CHECK_EQ(plane.yAxis().norm(), 2);
}

PR_TEST("Face Back")
{
	BoundingBox box(1, 2, 3);
	Plane plane = box.getFace(BoundingBox::FS_Back);
	PR_CHECK_EQ(plane.normal(), Eigen::Vector3f(0, 0, -1));
	PR_CHECK_NEARLY_EQ(plane.position()(2), 3 / 2.0f);
	PR_CHECK_EQ(plane.xAxis().norm(), 1);
	PR_CHECK_EQ(plane.yAxis().norm(), 2);
}

PR_TEST("Face Left")
{
	BoundingBox box(1, 2, 3);
	Plane plane = box.getFace(BoundingBox::FS_Left);
	PR_CHECK_EQ(plane.normal(), Eigen::Vector3f(1, 0, 0));
	PR_CHECK_NEARLY_EQ(plane.position()(0), -1 / 2.0f);
	PR_CHECK_EQ(plane.xAxis().norm(), 3);
	PR_CHECK_EQ(plane.yAxis().norm(), 2);
}

PR_TEST("Face Right")
{
	BoundingBox box(1, 2, 3);
	Plane plane = box.getFace(BoundingBox::FS_Right);
	PR_CHECK_EQ(plane.normal(), Eigen::Vector3f(-1, 0, 0));
	PR_CHECK_NEARLY_EQ(plane.position()(0), 1 / 2.0f);
	PR_CHECK_EQ(plane.xAxis().norm(), 3);
	PR_CHECK_EQ(plane.yAxis().norm(), 2);
}

PR_TEST("Face Top")
{
	BoundingBox box(1, 2, 3);
	Plane plane = box.getFace(BoundingBox::FS_Top);
	PR_CHECK_EQ(plane.normal(), Eigen::Vector3f(0, -1, 0));
	PR_CHECK_NEARLY_EQ(plane.position()(1), 1);
	PR_CHECK_EQ(plane.xAxis().norm(), 1);
	PR_CHECK_EQ(plane.yAxis().norm(), 3);
}

PR_TEST("Face Bottom")
{
	BoundingBox box(1, 2, 3);
	Plane plane = box.getFace(BoundingBox::FS_Bottom);
	PR_CHECK_EQ(plane.normal(), Eigen::Vector3f(0, 1, 0));
	PR_CHECK_NEARLY_EQ(plane.position()(1), -1);
	PR_CHECK_EQ(plane.xAxis().norm(), 1);
	PR_CHECK_EQ(plane.yAxis().norm(), 3);
}

PR_END_TESTCASE()

// MAIN
PRT_BEGIN_MAIN
PRT_TESTCASE(BoundingBox);
PRT_END_MAIN