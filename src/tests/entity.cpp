#include "entity/Entity.h"

#include "Test.h"

using namespace PR;

PR_BEGIN_TESTCASE(Entity)

PR_TEST("id")
{
	Entity entity(0, "Test");
	PR_CHECK_EQ(entity.id(), (uint32)0);
}

PR_TEST("name")
{
	Entity entity(0, "Test");
	PR_CHECK_EQ(entity.name(), "Test");
}

PR_TEST("type")
{
	Entity entity(0, "Test");
	PR_CHECK_EQ(entity.type(), "null");
}

PR_TEST("isRenderable")
{
	Entity entity(0, "Test");
	PR_CHECK_FALSE(entity.isRenderable());
}

PR_TEST("matrix")
{
	Entity entity(0, "Test");
	PR_CHECK_EQ(entity.transform().matrix(), Eigen::Matrix4f::Identity());
}

PR_TEST("direction matrix")
{
	Entity entity(0, "Test");
	entity.freeze(nullptr); // Exploit: Entity (not RenderEntity) does not make use of the parameter!
	PR_CHECK_EQ(entity.directionMatrix(), Eigen::Matrix3f::Identity());
}

PR_TEST("nonuniform scale")
{
	Entity::Transform trans;
    trans.fromPositionOrientationScale(Eigen::Vector3f(0, 1, 1), Eigen::Quaternionf(Eigen::AngleAxisf(0.5f*PR_PI, Eigen::Vector3f::UnitZ())), Eigen::Vector3f(1, 2, 1));
	//trans = Eigen::Translation3f(0,1,1) * Eigen::Quaternionf(Eigen::AngleAxisf(0.5f*PR_PI, Eigen::Vector3f::UnitZ())) * Eigen::Scaling(Eigen::Vector3f(1, 2, 1));

	Eigen::Vector3f pos(1, 1, 1);
	Eigen::Vector3f res = trans * pos;

	PR_CHECK_NEARLY_EQ(res, Eigen::Vector3f(-2, 2, 2));
}

PR_END_TESTCASE()

// MAIN
PRT_BEGIN_MAIN
PRT_TESTCASE(Entity);
PRT_END_MAIN