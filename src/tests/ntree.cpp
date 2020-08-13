#include "container/NTree.h"
#include "container/NTreeBuilder.h"

#include "Test.h"

using namespace PR;

PR_BEGIN_TESTCASE(NTree)
PR_TEST("QuadTree D0")
{
	using V = QuadTree<float>::UniformIndex;
	QuadTree<float>::Builder builder;

	builder.value(42);

	auto tree = builder.build();

	PR_CHECK_EQ(tree->maxDepth(), 0);
	PR_CHECK_EQ(tree->leafNodeCount(), 1);
	PR_CHECK_EQ(tree->branchNodeCount(), 0);
	PR_CHECK_EQ(tree->valueAt(V{ 0.5f, 0.5f }), 42);
}
PR_TEST("QuadTree D1")
{
	using V = QuadTree<float>::UniformIndex;
	QuadTree<float>::Builder builder;

	builder.begin();
	builder.value(16); // [0.0,0.5]x[0.0,0.5]
	builder.value(32); // [0.5,1.0]x[0.0,0.5]
	builder.value(48); // [0.0,0.5]x[0.5,1.0]
	builder.value(64); // [0.5,1.0]x[0.5,1.0]
	builder.end();

	auto tree = builder.build();

	PR_CHECK_EQ(tree->maxDepth(), 1);
	PR_CHECK_EQ(tree->leafNodeCount(), 4);
	PR_CHECK_EQ(tree->branchNodeCount(), 1);
	PR_CHECK_EQ(tree->valueAt(V{ 0.25f, 0.25f }), 16);
	PR_CHECK_EQ(tree->valueAt(V{ 0.55f, 0.25f }), 32);
	PR_CHECK_EQ(tree->valueAt(V{ 0.25f, 0.75f }), 48);
	PR_CHECK_EQ(tree->valueAt(V{ 0.85f, 0.65f }), 64);
}
PR_TEST("QuadTree D2")
{
	using V = QuadTree<float>::UniformIndex;
	QuadTree<float>::Builder builder;

	builder.begin();
	builder.value(16);	// [0.0,0.5]x[0.0,0.5]
	builder.begin();	// [0.5,1.0]x[0.0,0.5]
	builder.value(101); // > [0.5,0.75]x[0.0,0.25]
	builder.value(202); // > [0.75,1.0]x[0.0,0.25]
	builder.value(303); // > [0.5,0.75]x[0.25,0.5]
	builder.value(404); // > [0.75,1.0]x[0.25,0.5]
	builder.end();		// --------
	builder.value(48);	// [0.0,0.5]x[0.5,1.0]
	builder.value(64);	// [0.5,1.0]x[0.5,1.0]
	builder.end();

	auto tree = builder.build();

	PR_CHECK_EQ(tree->maxDepth(), 2);
	PR_CHECK_EQ(tree->leafNodeCount(), 7);
	PR_CHECK_EQ(tree->branchNodeCount(), 2);
	PR_CHECK_EQ(tree->valueAt(V{ 0.25f, 0.25f }), 16);
	PR_CHECK_EQ(tree->valueAt(V{ 0.55f, 0.20f }), 101);
	PR_CHECK_EQ(tree->valueAt(V{ 0.85f, 0.15f }), 202);
	PR_CHECK_EQ(tree->valueAt(V{ 0.55f, 0.35f }), 303);
	PR_CHECK_EQ(tree->valueAt(V{ 0.85f, 0.45f }), 404);
	PR_CHECK_EQ(tree->valueAt(V{ 0.25f, 0.75f }), 48);
	PR_CHECK_EQ(tree->valueAt(V{ 0.85f, 0.65f }), 64);
}
PR_END_TESTCASE()

// MAIN
PRT_BEGIN_MAIN
PRT_TESTCASE(NTree);
PRT_END_MAIN