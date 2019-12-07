#include "container/kdTreeCollider.h"
#include "Logger.h"

namespace PR {

/* TODO: Notify id to entities for debug purposes! */

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

static void loadNode(std::istream& stream, kdTreeCollider::kdNodeCollider*& node,
					 size_t& nodeCount)
{
	++nodeCount;
	uint32 id, leaf;
	stream >> id >> leaf;

	if (leaf) {
		kdTreeCollider::kdLeafNodeCollider* leafN = new kdTreeCollider::kdLeafNodeCollider(id);
		size_t count;
		stream >> count;
		leafN->objects.reserve(count);
		for (size_t i = 0; i < count; ++i) {
			uint64 entity;
			stream >> entity;
			leafN->objects.push_back(entity);
		}

		node = leafN;
	} else {
		uint32 axis;
		float splitPos;
		uint32 idLeft, idRight;
		stream >> axis >> splitPos >> idLeft >> idRight;

		kdTreeCollider::kdInnerNodeCollider* innerN
			= new kdTreeCollider::kdInnerNodeCollider(id, axis, splitPos, nullptr, nullptr);
		node = innerN;

		loadNode(stream, innerN->left, nodeCount);
		loadNode(stream, innerN->right, nodeCount);
	}
}

void kdTreeCollider::load(std::istream& stream)
{
	PR_ASSERT(mRoot == nullptr, "Expected to be an empty tree");

	std::string identifier;
	stream >> identifier;
	if (identifier != "pearray_kdtree") {
		PR_LOG(L_ERROR) << "Invalid kdtree file given!" << std::endl;
		return;
	}

	for (int i = 0; i < 3; ++i) {
		stream >> mBoundingBox.lowerBound()(i);
		stream >> mBoundingBox.upperBound()(i);
	}

	mNodeCount = 0;
	loadNode(stream, mRoot, mNodeCount);
}
} // namespace PR
