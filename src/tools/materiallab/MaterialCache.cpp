#include "MaterialCache.h"

MaterialCache::MaterialCache()
	: mData()
{
}

MaterialCache::~MaterialCache() {}

void MaterialCache::clear()
{
	mData.clear();
}

void MaterialCache::normalizeMaximum()
{
	const float norm = maximumEntry();
	if (norm <= PR::PR_EPSILON)
		return;

	const float inorm = 1 / norm;
	mData.visit([=](float& v) { v *= inorm; });
}

void MaterialCache::normalizeIntegral()
{
	const float norm = integral();
	if (norm <= PR::PR_EPSILON)
		return;

	const float inorm = 1 / norm;
	mData.visit([=](float& v) { v *= inorm; });
}

float MaterialCache::maximumEntry() const
{
	float max = 0.0f;
	mData.visit([&](float v) { max = std::max(max, v); });
	return max;
}

float MaterialCache::integral() const
{
	return 0;
}

void MaterialCache::get(std::vector<PR::Vector2f>& points, std::vector<float>& values)
{
	mData.enumerate([&](const Tree::UniformIndex& idx, float val) {
		points.push_back(PR::Vector2f(PR::PR_PI * idx(0), 2 * PR::PR_PI * idx(1)));
		values.push_back(val);
	});
}
