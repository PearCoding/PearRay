#include "BxDFContainer.h"
#include "BxDF.h"
#include "container/QuickSort.h"
#include "math/Projection.h"
#include "memory/MemoryStack.h"

namespace PR {
BxDFContainer::BxDFContainer(const ShadingPoint& spt)
	: mBxDFCount(0)
	, mNs(spt.N)
	, mNx(spt.Nx)
	, mNy(spt.Ny)
{
}

BxDFEval BxDFContainer::createEvalIO(size_t samples, MemoryStack& mem)
{
	BxDFEval eval;
	eval.Size		 = samples;
	eval.In.Outgoing = (Vector3f*)mem.allocate(samples * sizeof(Vector3f), alignof(Vector3f));
	eval.Out.Weight	 = (SpectralBlob*)mem.allocate(samples * sizeof(SpectralBlob), alignof(SpectralBlob));
	eval.Out.PDF_S	 = (float*)mem.allocate(samples * sizeof(float), alignof(float));
	return eval;
}

void BxDFContainer::eval(const BxDFEval& io, BxDFType type) const
{
	for (size_t s = 0; s < io.Size; ++s)
		io.In.Outgoing[s] = convertWorldToLocal(io.In.Outgoing[s]);

	std::fill_n(io.Out.Weight, io.Size, SpectralBlob::Zero());
	std::fill_n(io.Out.PDF_S, io.Size, 0);

	for (size_t i = 0; i < mBxDFCount; ++i) {
		if ((mBxDFs[i]->type() & type) == 0)
			continue;
		mBxDFs[i]->eval(io);
	}
}

BxDFSample BxDFContainer::createSampleIO(size_t samples, MemoryStack& mem)
{
	BxDFSample sample;
	sample.Size			= samples;
	sample.In.RND		= (Vector2f*)mem.allocate(samples * sizeof(Vector2f), alignof(Vector2f));
	sample.Out.Outgoing = (Vector3f*)mem.allocate(samples * sizeof(Vector3f), alignof(Vector3f));
	sample.Out.Weight	= (SpectralBlob*)mem.allocate(samples * sizeof(SpectralBlob), alignof(SpectralBlob));
	sample.Out.PDF_S	= (float*)mem.allocate(samples * sizeof(float), alignof(float));
	sample.Out.Type		= (BxDFType*)mem.allocate(samples * sizeof(BxDFType), alignof(BxDFType));
	return sample;
}

void BxDFContainer::sample(const BxDFSample& io, BxDFType type) const
{
	std::fill_n(io.Out.Weight, io.Size, SpectralBlob::Zero());
	std::fill_n(io.Out.PDF_S, io.Size, 0);
	std::fill_n(io.Out.Outgoing, io.Size, Vector3f(0, 0, 0));
	std::fill_n(io.Out.Type, io.Size, BT_None);

	const size_t count = bxdfCount(type);
	if (count == 0)
		return;

	size_t pickCount[MAX_BXDF_COUNT];
	std::fill_n(pickCount, count, 0);

	for (size_t s = 0; s < io.Size; ++s) {
		float u			 = io.In.RND[s].x();
		uint8 pick		 = Projection::map(u, 0UL, count - 1);
		io.In.RND[s].x() = std::min(u * count - pick, PR_EPSILON); // Remap
		++pickCount[pick];
	}

	// We make use of a special property -> The order of the samples is not important.
	// Therefore we do not care about the sample association to one particular bxdf.
	size_t currentPos = 0;
	for (size_t i = 0; i < mBxDFCount; ++i) {
		const BxDFType bxdftype = mBxDFs[i]->type();
		if ((bxdftype & type) == 0 || pickCount[i] == 0)
			continue;

		BxDFSample innerIO;
		innerIO.Size		 = pickCount[i];
		innerIO.In.RND		 = io.In.RND + currentPos;
		innerIO.Out.Weight	 = io.Out.Weight + currentPos;
		innerIO.Out.PDF_S	 = io.Out.PDF_S + currentPos;
		innerIO.Out.Outgoing = io.Out.Outgoing + currentPos;
		innerIO.Out.Type	 = io.Out.Type + currentPos;

		mBxDFs[i]->sample(innerIO);

		std::fill_n(innerIO.Out.Type, pickCount, bxdftype);
		currentPos += pickCount[i];
	}

	PR_ASSERT(currentPos == io.Size, "Expected split size to converge to full size");

	for (size_t s = 0; s < io.Size; ++s)
		io.Out.Outgoing[s] = convertLocalToWorld(io.Out.Outgoing[s]);
}

size_t BxDFContainer::bxdfCount(BxDFType type) const
{
	size_t num = 0;
	for (size_t i = 0; i < mBxDFCount; ++i) {
		if (mBxDFs[i]->type() & type)
			++num;
	}
	return num;
}

void BxDFContainer::add(BxDF* bxdf)
{
	PR_ASSERT(bxdf, "Expected valid BxDF");
	PR_ASSERT(mBxDFCount < MAX_BXDF_COUNT, "Too many BxDFs added");
	mBxDFs[mBxDFCount] = bxdf;
	++mBxDFCount;
}

} // namespace PR