#include "entity/VirtualEntity.h"

#include "Test.h"

using namespace PR;

PR_BEGIN_TESTCASE(VirtualEntity)

PR_TEST("id")
{
	VirtualEntity entity(0, "Test");
	PR_CHECK_EQ(entity.id(), (uint32)0);
}

PR_TEST("name")
{
	VirtualEntity entity(0, "Test");
	PR_CHECK_EQ(entity.name(), "Test");
}

PR_TEST("type")
{
	VirtualEntity entity(0, "Test");
	PR_CHECK_EQ(entity.type(), "null");
}

PR_TEST("isRenderable")
{
	VirtualEntity entity(0, "Test");
	PR_CHECK_FALSE(entity.isRenderable());
}

PR_TEST("matrix")
{
	VirtualEntity entity(0, "Test");
	PR_CHECK_EQ(entity.transform().matrix(), Eigen::Matrix4f::Identity());
}

PR_TEST("normal matrix")
{
	VirtualEntity entity(0, "Test");
	entity.freeze(nullptr); // Exploit: VirtualEntity (not IEntity) does not make use of the parameter!
	PR_CHECK_EQ(entity.normalMatrix(), Eigen::Matrix3f::Identity());
}

PR_TEST("nonuniform scale")
{
	VirtualEntity::Transform trans;
    trans.fromPositionOrientationScale(Vector3f(0, 1, 1), Eigen::Quaternionf(Eigen::AngleAxisf(0.5f*PR_PI, Vector3f::UnitZ())), Vector3f(1, 2, 1));

	Vector3f pos(1, 1, 1);
	Vector3f res = trans * pos;

	PR_CHECK_NEARLY_EQ(res, Vector3f(-2, 2, 2));
}

PR_END_TESTCASE()

// MAIN
PRT_BEGIN_MAIN
PRT_TESTCASE(VirtualEntity);
PRT_END_MAIN