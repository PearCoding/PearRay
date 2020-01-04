#include "container/kdTreeCollider.h"
#include "Logger.h"
#include "serialization/Serializer.h"

namespace PR {

kdTreeCollider::kdTreeCollider()
	: mRoot(nullptr)
	, mNodeCount(0)
{
}

////////////////////////////////////////

static void deleteNode(kdTreeCollider::kdNodeCollider* node)
{
	if (node) {
		if (node->isLeaf)
			delete (kdTreeCollider::kdLeafNodeCollider*)node;
		else {
			deleteNode(((kdTreeCollider::kdInnerNodeCollider*)node)->left);
			deleteNode(((kdTreeCollider::kdInnerNodeCollider*)node)->right);
			delete (kdTreeCollider::kdInnerNodeCollider*)node;
		}
	}
}

kdTreeCollider::~kdTreeCollider()
{
	deleteNode(mRoot);
}

////////////////////////////////////////

static void loadNode(Serializer& stream, kdTreeCollider::kdNodeCollider*& node,
					 size_t& nodeCount)
{
	++nodeCount;
	uint8 leaf = 0;
	stream | leaf;

	if (leaf) {
		kdTreeCollider::kdLeafNodeCollider* leafN = new kdTreeCollider::kdLeafNodeCollider();
		stream | leafN->objects;
		node = leafN;
	} else {
		uint8 axis;
		float splitPos;
		kdTreeCollider::NodeID idLeft, idRight;
		stream | axis | splitPos | idLeft | idRight;

		kdTreeCollider::kdInnerNodeCollider* innerN
			= new kdTreeCollider::kdInnerNodeCollider(axis, splitPos, nullptr, nullptr);
		node = innerN;

		loadNode(stream, innerN->left, nodeCount);
		loadNode(stream, innerN->right, nodeCount);
	}
}

void kdTreeCollider::load(Serializer& stream)
{
	PR_ASSERT(stream.isReadMode(), "Expected stream to be in read mode");
	PR_ASSERT(mRoot == nullptr, "Expected to be an empty tree");

	mNodeCount = 0;

	std::string identifier;
	stream | identifier;
	if (identifier != "pearray_kdtree") {
		PR_LOG(L_ERROR) << "Invalid kdtree file given!" << std::endl;
		return;
	}

	if (!stream.isValid())
		return;

	stream | mBoundingBox.lowerBound();
	stream | mBoundingBox.upperBound();

	loadNode(stream, mRoot, mNodeCount);
}
} // namespace PR
