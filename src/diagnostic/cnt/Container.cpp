#include "Container.h"

#include <QTextStream>

struct Node {
	Node(unsigned int id, bool leaf)
		: id(id)
		, isLeaf(leaf)
	{
	}

	const unsigned int id;
	const bool isLeaf;
};

struct InnerNode : public Node {
	InnerNode(unsigned int id, unsigned char axis, float sp,
			  Node* l, Node* r)
		: Node(id, false)
		, axis(axis)
		, splitPos(sp)
		, left(l)
		, right(r)
	{
	}

	const unsigned char axis;
	const float splitPos;
	Node* left;
	Node* right;
};

struct LeafNode : public Node {
	explicit LeafNode(unsigned int id)
		: Node(id, true)
	{
	}
};

Container::Container()
	: mDepth(0)
	, mNodeCount(0)
	, mInnerCount(0)
	, mRoot(nullptr)
{
}

static void deleteNode(Node* node)
{
	if (node) {
		if (node->isLeaf)
			delete reinterpret_cast<LeafNode*>(node);
		else {
			deleteNode(reinterpret_cast<InnerNode*>(node)->left);
			deleteNode(reinterpret_cast<InnerNode*>(node)->right);
			delete reinterpret_cast<InnerNode*>(node);
		}
	}
}

Container::~Container()
{
	if (mRoot)
		deleteNode(mRoot);
}

static void loadNode(QTextStream& stream, Node*& node, size_t depth,
					 unsigned int& nodeCount, unsigned int& innerCount, unsigned int& maxDepth)
{
	if (depth > maxDepth)
		maxDepth = depth;

	nodeCount += 1;

	unsigned int id, leaf;
	stream >> id >> leaf;
	if (leaf) {
		LeafNode* leafN = new LeafNode(id);
		size_t count;
		stream >> count;
		// Skip entries
		for (size_t i = 0; i < count; ++i) {
			unsigned long long entity;
			stream >> entity;
		}

		node = leafN;
	} else {
		innerCount += 1;

		unsigned int axis;
		float splitPos;
		unsigned int idLeft, idRight;
		stream >> axis >> splitPos >> idLeft >> idRight;

		InnerNode* innerN
			= new InnerNode(id, axis, splitPos, nullptr, nullptr);
		node = innerN;

		loadNode(stream, innerN->left, depth + 1, nodeCount, innerCount, maxDepth);
		loadNode(stream, innerN->right, depth + 1, nodeCount, innerCount, maxDepth);
	}
}

bool Container::load(QFile& file)
{
	if (mRoot) {
		deleteNode(mRoot);
		mRoot = nullptr;
	}

	if(!file.open(QIODevice::ReadOnly))
		return false;

	QTextStream stream(&file);

	mNodeCount  = 0;
	mInnerCount = 0;
	mDepth		= 0;

	QString identifier;
	stream >> identifier;
	if (identifier != "pearray_kdtree") {
		return false;
	}

	for (int i = 0; i < 3; ++i) {
		stream >> mLowerBound[i];
		stream >> mUpperBound[i];
	}

	loadNode(stream, mRoot, 0, mNodeCount, mInnerCount, mDepth);

	return true;
}

/*
 * p1 -------- p2
 *  |          |
 *  |          |
 *  |          |
 *  |          |
 * p3 -------- p4
 */
static void drawPlane(QVector<QVector3D>& vertices, QVector<unsigned int>& indices,
					  const QVector3D& p1, const QVector3D& p2,
					  const QVector3D& p3, const QVector3D& p4)
{
	unsigned int index = vertices.size();
	vertices.push_back(p1);
	vertices.push_back(p2);
	vertices.push_back(p3);
	vertices.push_back(p4);

	// p1 - p2
	indices.push_back(index);
	indices.push_back(index + 1);
	// p1 - p3
	indices.push_back(index);
	indices.push_back(index + 2);
	// p2 - p4
	indices.push_back(index + 1);
	indices.push_back(index + 3);
	// p3 - p4
	indices.push_back(index + 2);
	indices.push_back(index + 3);
}

static void drawNode(QVector<QVector3D>& vertices, QVector<unsigned int>& indices,
					 Node* node, size_t depth, unsigned int maxDepth,
					 const QVector3D& lowerBound, const QVector3D& upperBound)
{
	if (depth > maxDepth)
		return;

	if (node->isLeaf) {
		// skip
	} else {
		InnerNode* innerN = reinterpret_cast<InnerNode*>(node);

		switch (innerN->axis) {
		default:
		case 0:
			drawPlane(vertices, indices,
					  QVector3D(innerN->splitPos, lowerBound[1], lowerBound[2]),
					  QVector3D(innerN->splitPos, upperBound[1], lowerBound[2]),
					  QVector3D(innerN->splitPos, lowerBound[1], upperBound[2]),
					  QVector3D(innerN->splitPos, upperBound[1], upperBound[2]));
			break;
		case 1:
			drawPlane(vertices, indices,
					  QVector3D(lowerBound[0], innerN->splitPos, lowerBound[2]),
					  QVector3D(upperBound[0], innerN->splitPos, lowerBound[2]),
					  QVector3D(lowerBound[0], innerN->splitPos, upperBound[2]),
					  QVector3D(upperBound[0], innerN->splitPos, upperBound[2]));
			break;
		case 2:
			drawPlane(vertices, indices,
					  QVector3D(lowerBound[0], lowerBound[1], innerN->splitPos),
					  QVector3D(upperBound[0], lowerBound[1], innerN->splitPos),
					  QVector3D(lowerBound[0], upperBound[1], innerN->splitPos),
					  QVector3D(upperBound[0], upperBound[1], innerN->splitPos));
			break;
		}

		QVector3D upSplit	  = upperBound;
		QVector3D lowSplit	 = lowerBound;
		upSplit[innerN->axis]  = innerN->splitPos;
		lowSplit[innerN->axis] = innerN->splitPos;

		drawNode(vertices, indices, innerN->left, depth + 1, maxDepth, lowerBound, upSplit);
		drawNode(vertices, indices, innerN->right, depth + 1, maxDepth, lowSplit, upperBound);
	}
}
void Container::populate(QVector<QVector3D>& vertices, QVector<unsigned int>& indices, int maxDepth) const
{
	vertices.clear();
	indices.clear();

	vertices.reserve(4 * mInnerCount + 8);
	indices.reserve(8 * mInnerCount + 12);

	// Bounding Box
	drawPlane(vertices, indices,
			  QVector3D(mLowerBound[0], mLowerBound[1], mLowerBound[2]),
			  QVector3D(mUpperBound[0], mLowerBound[1], mLowerBound[2]),
			  QVector3D(mLowerBound[0], mUpperBound[1], mLowerBound[2]),
			  QVector3D(mUpperBound[0], mUpperBound[1], mLowerBound[2]));
	drawPlane(vertices, indices,
			  QVector3D(mLowerBound[0], mLowerBound[1], mUpperBound[2]),
			  QVector3D(mUpperBound[0], mLowerBound[1], mUpperBound[2]),
			  QVector3D(mLowerBound[0], mUpperBound[1], mUpperBound[2]),
			  QVector3D(mUpperBound[0], mUpperBound[1], mUpperBound[2]));

	indices.push_back(0);
	indices.push_back(4);

	indices.push_back(1);
	indices.push_back(5);

	indices.push_back(2);
	indices.push_back(6);

	indices.push_back(3);
	indices.push_back(7);

	// Nodes
	drawNode(vertices, indices, mRoot, 1, maxDepth, mLowerBound, mUpperBound);
}
