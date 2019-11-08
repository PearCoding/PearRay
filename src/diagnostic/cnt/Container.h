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

	inline unsigned int depth() const
	{
		return mDepth;
	}

	inline unsigned int nodeCount() const
	{
		return mNodeCount;
	}

    inline unsigned int innerNodeCount() const
    {
        return mInnerCount;
    }

	bool load(QFile& file);

    void populate(QVector<QVector3D>& vertices, QVector<unsigned int>& indices, int maxDepth) const;

private:
	unsigned int mDepth;
	unsigned int mNodeCount;
    unsigned int mInnerCount;
	struct Node* mRoot;
	QVector3D mLowerBound;
	QVector3D mUpperBound;
};