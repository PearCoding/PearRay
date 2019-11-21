#pragma once

#include <QVector>
#include <QVector3D>
#include <QFile>

/* kdTree Node container! */
class Container {
public:
	Container();
	virtual ~Container();

	inline bool isEmpty() const
	{
		return mRoot == nullptr;
	}

	inline float lowerBound(int i) const
	{
		return mLowerBound[i];
	}

	inline float upperBound(int i) const
	{
		return mUpperBound[i];
	}

	inline size_t depth() const
	{
		return mDepth;
	}

	inline size_t nodeCount() const
	{
		return mNodeCount;
	}

    inline size_t innerNodeCount() const
    {
        return mInnerCount;
    }

	bool load(QFile& file);

    void populate(QVector<QVector3D>& vertices, QVector<unsigned int>& indices, size_t maxDepth) const;

private:
	size_t mDepth;
	size_t mNodeCount;
	size_t mInnerCount;
	struct Node* mRoot;
	QVector3D mLowerBound;
	QVector3D mUpperBound;
};