#pragma once

#include "container/NTree.h"

/// Naive material cache
class MaterialCache {
public:
	MaterialCache();
	virtual ~MaterialCache();

	inline float value(float theta, float phi) const
	{
		return mData.valueAt(index(theta, phi));
	}

	inline float nearestValue(float theta, float phi) const
	{
		return mData.nearestValueAt(index(theta, phi));
	}

	inline void addValue(float theta, float phi, float val)
	{
		mData.insertAt(index(theta, phi), val);
	}

	void get(std::vector<PR::Vector2f>& points, std::vector<float>& values);

	void clear();
	void normalizeMaximum();
	void normalizeIntegral();

	float maximumEntry() const;
	float integral() const;

private:
	using Tree = PR::QuadTree<float>;

	inline Tree::UniformIndex index(float theta, float phi) const
	{
		float u = std::max(0.0f, theta / PR::PR_PI);
		float v = std::max(0.0f, phi / (2 * PR::PR_PI));
		return Tree::UniformIndex(u, v);
	}

	PR::QuadTree<float> mData;
};