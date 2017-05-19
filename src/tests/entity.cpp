#include "entity/Entity.h"

#include "Test.h"

using namespace PR;

PR_BEGIN_TESTCASE(Entity)

PR_TEST("id")
{
    Entity entity(0, "Test");
    PR_CHECK_EQ(entity.id(), 0);
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

PR_TEST("position")
{
    Entity entity(0, "Test");
    PR_CHECK_EQ(entity.position(), Eigen::Vector3f(0,0,0));
}

PR_TEST("scale")
{
    Entity entity(0, "Test");
    PR_CHECK_EQ(entity.scale(), Eigen::Vector3f(1,1,1));
}

PR_TEST("rotation")
{
    Entity entity(0, "Test");
    PR_CHECK_EQ(entity.rotation(), Eigen::Quaternionf::Identity());
}

PR_TEST("matrix")
{
    Entity entity(0, "Test");
    PR_CHECK_EQ(entity.transform().matrix(), Eigen::Matrix4f::Identity());
}

PR_TEST("direction matrix")
{
    Entity entity(0, "Test");
    PR_CHECK_EQ(entity.directionMatrix(), Eigen::Matrix3f::Identity());
}

PR_END_TESTCASE()

// MAIN
PRT_BEGIN_MAIN
PRT_TESTCASE(Entity);
PRT_END_MAIN