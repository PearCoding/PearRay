#pragma once

#include "BxDFData.h"
#include "BxDFType.h"
#include "math/Tangent.h"
#include "shader/ShadingPoint.h"

namespace PR {
class BxDF;
class MemoryStack;

// Container of BxDF lopes
class PR_LIB BxDFContainer {
public:
	explicit BxDFContainer(const ShadingPoint& spt);

	static BxDFEval createEvalIO(size_t samples, MemoryStack& mem);
	void eval(const BxDFEval& s, BxDFType type = BT_All) const;
	static BxDFSample createSampleIO(size_t samples, MemoryStack& mem);
	void sample(const BxDFSample& s, BxDFType type = BT_All) const;

	inline Vector3f convertWorldToLocal(const Vector3f& V) const
	{
		return Tangent::toTangentSpace(mNs, mNx, mNy, V);
	}

	inline Vector3f convertLocalToWorld(const Vector3f& V) const
	{
		return Tangent::fromTangentSpace(mNs, mNx, mNy, V);
	}

	size_t bxdfCount(BxDFType type = BT_All) const;
	void add(BxDF* bxdf);

private:
	static constexpr size_t MAX_BXDF_COUNT = 16;
	std::array<BxDF*, MAX_BXDF_COUNT> mBxDFs;
	size_t mBxDFCount = 0;

	const Vector3f mNs;
	const Vector3f mNx;
	const Vector3f mNy;
};
} // namespace PR