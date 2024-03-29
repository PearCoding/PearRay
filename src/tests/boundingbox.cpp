#include "geometry/BoundingBox.h"
#include "geometry/Plane.h"
#include "trace/HitPoint.h"

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
	PR_CHECK_EQ(box.lowerBound(), Vector3f(-1, -1, -1));
	PR_CHECK_EQ(box.upperBound(), Vector3f(1, 1, 1));
	PR_CHECK_EQ(box.center(), Vector3f(0, 0, 0));
}

PR_TEST("Contains")
{
	BoundingBox box(2, 2, 2);
	PR_CHECK_TRUE(box.contains(Vector3f(0, 0, 0)));
	PR_CHECK_FALSE(box.contains(Vector3f(2, 2, 2)));
	PR_CHECK_FALSE(box.contains(Vector3f(-2, 0, 3)));
}

PR_TEST("Clip Inside")
{
	BoundingBox box(2, 2, 2);
	BoundingBox parent(3, 3, 3);

	box.clipBy(parent);
	PR_CHECK_EQ(box.width(), 2);
	PR_CHECK_EQ(box.height(), 2);
	PR_CHECK_EQ(box.depth(), 2);
	PR_CHECK_EQ(box.center(), Vector3f(0, 0, 0));
}

PR_TEST("Clip Outside")
{
	BoundingBox box(2, 2, 2);
	BoundingBox parent(1, 1, 1);

	box.clipBy(parent);
	PR_CHECK_EQ(box.width(), 1);
	PR_CHECK_EQ(box.height(), 1);
	PR_CHECK_EQ(box.depth(), 1);
	PR_CHECK_EQ(box.center(), Vector3f(0, 0, 0));
}

PR_TEST("Clip Zero")
{
	BoundingBox box(Vector3f(0, 0, 0), Vector3f(1, 1, 1));
	BoundingBox parent(Vector3f(-1, -1, -1), Vector3f(0, 0, 0));

	box.clipBy(parent);
	PR_CHECK_EQ(box.width(), 0);
	PR_CHECK_EQ(box.height(), 0);
	PR_CHECK_EQ(box.depth(), 0);
	PR_CHECK_EQ(box.lowerBound(), Vector3f(0, 0, 0));
	PR_CHECK_EQ(box.upperBound(), Vector3f(0, 0, 0));
}

PR_TEST("Clip Left Half")
{
	BoundingBox box(Vector3f(0, 1, 2), Vector3f(2, 3, 4));
	BoundingBox parent(Vector3f(-1, 0, 1), Vector3f(1, 2, 3));

	box.clipBy(parent);
	PR_CHECK_EQ(box.width(), 1);
	PR_CHECK_EQ(box.height(), 1);
	PR_CHECK_EQ(box.depth(), 1);
	PR_CHECK_EQ(box.lowerBound(), Vector3f(0, 1, 2));
	PR_CHECK_EQ(box.upperBound(), Vector3f(1, 2, 3));
}

PR_TEST("Clip Right Half")
{
	BoundingBox box(Vector3f(-2, -3, -4), Vector3f(1, 0, -1));
	BoundingBox parent(Vector3f(-1, -2, -3), Vector3f(1, 2, 3));

	box.clipBy(parent);
	PR_CHECK_EQ(box.width(), 2);
	PR_CHECK_EQ(box.height(), 2);
	PR_CHECK_EQ(box.depth(), 2);
	PR_CHECK_EQ(box.lowerBound(), Vector3f(-1, -2, -3));
	PR_CHECK_EQ(box.upperBound(), Vector3f(1, 0, -1));
}

PR_TEST("Inflate")
{
	BoundingBox box(0, 0, 0);
	box.inflate(0.1f);
	PR_CHECK_EQ(box.lowerBound(), Vector3f(-0.1f, -0.1f, -0.1f));
	PR_CHECK_EQ(box.upperBound(), Vector3f(0.1f, 0.1f, 0.1f));
	PR_CHECK_EQ(box.center(), Vector3f(0, 0, 0));
}

PR_TEST("Inflate Max")
{
	BoundingBox box(0, 0, 0);
	box.inflate(0.1f, true);
	PR_CHECK_EQ(box.lowerBound(), Vector3f(0, 0, 0));
	PR_CHECK_EQ(box.upperBound(), Vector3f(0.1f, 0.1f, 0.1f));
	PR_CHECK_EQ(box.center(), Vector3f(0.05f, 0.05f, 0.05f));
}

PR_TEST("Intersects Left")
{
	BoundingBox box(2, 2, 2);
	Ray ray(Vector3f(-2, 0, 0),
			Vector3f(1, 0, 0));

	HitPoint s;
	box.intersects(ray, s);
	PR_CHECK_EQ(s.HitDistance, 1);

	Vector3f p = ray.t(s.HitDistance);
	PR_CHECK_EQ(box.getIntersectionSide(p), BoundingBox::FaceSide::Left);
}

PR_TEST("Intersects Right Inside")
{
	BoundingBox box(2, 2, 2);
	Ray ray(Vector3f(0, 0, 0),
			Vector3f(1, 0, 0));

	HitPoint s;
	box.intersects(ray, s);
	PR_CHECK_EQ(s.HitDistance, 1);

	Vector3f p = ray.t(s.HitDistance);
	PR_CHECK_EQ(box.getIntersectionSide(p), BoundingBox::FaceSide::Right);
}

PR_TEST("Intersects Right")
{
	BoundingBox box(2, 2, 2);
	Ray ray(Vector3f(2, 0, 0),
			Vector3f(-1, 0, 0));

	HitPoint s;
	box.intersects(ray, s);
	PR_CHECK_EQ(s.HitDistance, 1);

	Vector3f p = ray.t(s.HitDistance);
	PR_CHECK_EQ(box.getIntersectionSide(p), BoundingBox::FaceSide::Right);
}

PR_TEST("Intersects Front")
{
	BoundingBox box(2, 2, 2);
	Ray ray(Vector3f(0, 0, -2),
			Vector3f(0, 0, 1));

	HitPoint s;
	box.intersects(ray, s);
	PR_CHECK_EQ(s.HitDistance, 1);

	Vector3f p = ray.t(s.HitDistance);
	PR_CHECK_EQ(box.getIntersectionSide(p), BoundingBox::FaceSide::Front);
}

PR_TEST("Intersects Back Inside")
{
	BoundingBox box(2, 2, 2);
	Ray ray(Vector3f(0, 0, 0),
			Vector3f(0, 0, 1));

	HitPoint s;
	box.intersects(ray, s);
	PR_CHECK_EQ(s.HitDistance, 1);

	Vector3f p = ray.t(s.HitDistance);
	PR_CHECK_EQ(box.getIntersectionSide(p), BoundingBox::FaceSide::Back);
}

PR_TEST("Intersects Back")
{
	BoundingBox box(2, 2, 2);
	Ray ray(Vector3f(0, 0, 2),
			Vector3f(0, 0, -1));

	HitPoint s;
	box.intersects(ray, s);
	PR_CHECK_EQ(s.HitDistance, 1);

	Vector3f p = ray.t(s.HitDistance);
	PR_CHECK_EQ(box.getIntersectionSide(p), BoundingBox::FaceSide::Back);
}

PR_TEST("Intersects Bottom")
{
	BoundingBox box(2, 2, 2);
	Ray ray(Vector3f(0, -2, 0),
			Vector3f(0, 1, 0));

	HitPoint s;
	box.intersects(ray, s);
	PR_CHECK_EQ(s.HitDistance, 1);

	Vector3f p = ray.t(s.HitDistance);
	PR_CHECK_EQ(box.getIntersectionSide(p), BoundingBox::FaceSide::Bottom);
}

PR_TEST("Intersects Top Inside")
{
	BoundingBox box(2, 2, 2);
	Ray ray(Vector3f(0, 0, 0),
			Vector3f(0, 1, 0));

	HitPoint s;
	box.intersects(ray, s);
	PR_CHECK_EQ(s.HitDistance, 1);

	Vector3f p = ray.t(s.HitDistance);
	PR_CHECK_EQ(box.getIntersectionSide(p), BoundingBox::FaceSide::Top);
}

PR_TEST("Intersects Top")
{
	BoundingBox box(2, 2, 2);
	Ray ray(Vector3f(0, 2, 0),
			Vector3f(0, -1, 0));

	HitPoint s;
	box.intersects(ray, s);
	PR_CHECK_EQ(s.HitDistance, 1);

	Vector3f p = ray.t(s.HitDistance);
	PR_CHECK_EQ(box.getIntersectionSide(p), BoundingBox::FaceSide::Top);
}

PR_TEST("Intersects Complex")
{
	Vector3f point = Vector3f(0, 1, 0);
	BoundingBox box(2, 2, 2);
	Ray ray(Vector3f(1, 2, 0),
			Vector3f(-1, -1, 0));

	HitPoint s;
	box.intersects(ray, s);
	PR_CHECK_TRUE(s.HitDistance < PR_INF);

	Vector3f p = ray.t(s.HitDistance);
	PR_CHECK_NEARLY_EQ(p, point);
	PR_CHECK_EQ(box.getIntersectionSide(p), BoundingBox::FaceSide::Top);
}

PR_TEST("Intersects Range")
{
	BoundingBox box(2, 2, 2);
	Ray ray(Vector3f(-2, 0, 0),
			Vector3f(1, 0, 0));

	BoundingBox::IntersectionRange s = box.intersectsRange(ray);
	PR_CHECK_TRUE(s.Successful);
	PR_CHECK_EQ(s.Entry, 1);
	PR_CHECK_EQ(s.Exit, 3);
}

PR_TEST("Face Front")
{
	BoundingBox box(1, 2, 3);
	Plane plane = box.getFace(BoundingBox::FaceSide::Front);
	PR_CHECK_EQ(plane.normal(), Vector3f(0, 0, 1));
	PR_CHECK_NEARLY_EQ(plane.position()(2), -3 / 2.0f);
	PR_CHECK_EQ(plane.xAxis().norm(), 1);
	PR_CHECK_EQ(plane.yAxis().norm(), 2);
}

PR_TEST("Face Back")
{
	BoundingBox box(1, 2, 3);
	Plane plane = box.getFace(BoundingBox::FaceSide::Back);
	PR_CHECK_EQ(plane.normal(), Vector3f(0, 0, -1));
	PR_CHECK_NEARLY_EQ(plane.position()(2), 3 / 2.0f);
	PR_CHECK_EQ(plane.xAxis().norm(), 1);
	PR_CHECK_EQ(plane.yAxis().norm(), 2);
}

PR_TEST("Face Left")
{
	BoundingBox box(1, 2, 3);
	Plane plane = box.getFace(BoundingBox::FaceSide::Left);
	PR_CHECK_EQ(plane.normal(), Vector3f(1, 0, 0));
	PR_CHECK_NEARLY_EQ(plane.position()(0), -1 / 2.0f);
	PR_CHECK_EQ(plane.xAxis().norm(), 3);
	PR_CHECK_EQ(plane.yAxis().norm(), 2);
}

PR_TEST("Face Right")
{
	BoundingBox box(1, 2, 3);
	Plane plane = box.getFace(BoundingBox::FaceSide::Right);
	PR_CHECK_EQ(plane.normal(), Vector3f(-1, 0, 0));
	PR_CHECK_NEARLY_EQ(plane.position()(0), 1 / 2.0f);
	PR_CHECK_EQ(plane.xAxis().norm(), 3);
	PR_CHECK_EQ(plane.yAxis().norm(), 2);
}

PR_TEST("Face Top")
{
	BoundingBox box(1, 2, 3);
	Plane plane = box.getFace(BoundingBox::FaceSide::Top);
	PR_CHECK_EQ(plane.normal(), Vector3f(0, -1, 0));
	PR_CHECK_NEARLY_EQ(plane.position()(1), 1);
	PR_CHECK_EQ(plane.xAxis().norm(), 1);
	PR_CHECK_EQ(plane.yAxis().norm(), 3);
}

PR_TEST("Face Bottom")
{
	BoundingBox box(1, 2, 3);
	Plane plane = box.getFace(BoundingBox::FaceSide::Bottom);
	PR_CHECK_EQ(plane.normal(), Vector3f(0, 1, 0));
	PR_CHECK_NEARLY_EQ(plane.position()(1), -1);
	PR_CHECK_EQ(plane.xAxis().norm(), 1);
	PR_CHECK_EQ(plane.yAxis().norm(), 3);
}

PR_END_TESTCASE()

// MAIN
PRT_BEGIN_MAIN
PRT_TESTCASE(BoundingBox);
PRT_END_MAIN