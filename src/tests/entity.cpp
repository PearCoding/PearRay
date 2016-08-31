#include "entity/Entity.h"

#include "Test.h"

using namespace PR;

PR_BEGIN_TESTCASE(Entity)
PR_TEST("Parent");
{
	Entity* root = new Entity("Root");
	Entity* child = new Entity("Child", root);

	root->setPosition(PM::pm_Set(0, 1, 0, 1));
	child->setPosition(PM::pm_Set(0, 2, 0, 1));

	PR_CHECK_NEARLY_EQ_3(child->worldPosition(), PM::pm_Set(0, 3, 0, 1));
	PR_CHECK_NEARLY_EQ_3(child->position(), PM::pm_Set(0, 2, 0, 1));

	root->setRotation(PM::pm_RotationQuatRollPitchYaw(PM::pm_Set(PM::pm_DegToRad(90), 0, 0)));
	PR_CHECK_NEARLY_EQ_3(child->worldPosition(), PM::pm_Set(0, 1, 2, 1));
	PR_CHECK_NEARLY_EQ_3(root->worldPosition(), PM::pm_Set(0, 1, 0, 1));
	PR_CHECK_NEARLY_EQ_3(child->position(), PM::pm_Set(0, 2, 0, 1));

	root->setScale(PM::pm_Set(2, 1, 4));
	PR_CHECK_NEARLY_EQ_3(child->worldPosition(), PM::pm_Set(0, 1, 2, 1));
	PR_CHECK_NEARLY_EQ_3(child->worldScale(), PM::pm_Set(2, 1, 4, 0));

	root->setScale(PM::pm_Set(2, 2, 2));
	PR_CHECK_NEARLY_EQ_3(child->worldPosition(), PM::pm_Set(0, 1, 4, 1));

	root->setRotation(PM::pm_RotationQuatRollPitchYaw(PM::pm_Set(0, PM::pm_DegToRad(90), 0)));
	PR_CHECK_NEARLY_EQ_3(child->worldPosition(), PM::pm_Set(0, 5, 0, 1));

	root->setRotation(PM::pm_RotationQuatRollPitchYaw(PM::pm_Set(0, 0, PM::pm_DegToRad(90))));
	PR_CHECK_NEARLY_EQ_3(child->worldPosition(), PM::pm_Set(-4, 1, 0, 1));

	delete child;
	delete root;
}

PR_END_TESTCASE()

// MAIN
PRT_BEGIN_MAIN
PRT_TESTCASE(Entity);
PRT_END_MAIN