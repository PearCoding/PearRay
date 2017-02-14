#include "OutputMap.h"
#include "RenderContext.h"
#include "shader/ShaderClosure.h"
#include "spectral/Spectrum.h"
#include "material/Material.h"

#include "Diagnosis.h"

namespace PR
{
	OutputMap::OutputMap(RenderContext* renderer) :
		mRenderer(renderer)
	{
		mSpectral = new OutputSpectral(mRenderer);
		std::fill_n(mInt1D, V_1D_COUNT, nullptr);
		std::fill_n(mIntCounter, V_COUNTER_COUNT, nullptr);
		std::fill_n(mInt3D, V_3D_COUNT, nullptr);
	}

	OutputMap::~OutputMap()
	{
		deinit();
		delete mSpectral;
	}

	void OutputMap::init()
	{
		// Init outputs
		mSpectral->init();
		if(!mIntCounter[V_Samples])
			mIntCounter[V_Samples] = new OutputCounter(mRenderer, 0);

		if(!mInt1D[V_Quality] && mRenderer->settings().isAdaptiveSampling())
			mInt1D[V_Quality] = new Output1D(mRenderer, 0.0f);

		PM::avec3 zeroV = {0,0,0};
		for(uint32 i = 0; i < V_1D_COUNT; ++i)
		{
			if(mInt1D[i])
				mInt1D[i]->init();
		}

		for(uint32 i = 0; i < V_COUNTER_COUNT; ++i)
		{
			if(mIntCounter[i])
				mIntCounter[i]->init();
		}

		for(uint32 i = 0; i < V_3D_COUNT; ++i)
		{
			if(mInt3D[i])
				mInt3D[i]->init();
		}

		for(const auto& p: mCustom1D)
			p.second->init();

		for(const auto& p: mCustomCounter)
			p.second->init();

		for(const auto& p: mCustom3D)
			p.second->init();

		for(const auto& p: mCustomSpectral)
			p.second->init();

		// if(mInt1D[V_Quality])
		// 	mInt1D[V_Quality]->fill(std::numeric_limits<float>::max());
	}

	void OutputMap::deinit()
	{
		mSpectral->deinit();

		for(uint32 i = 0; i < V_1D_COUNT; ++i)
		{
			if(mInt1D[i])
			{
				mInt1D[i]->deinit();
				delete mInt1D[i];
				mInt1D[i] = nullptr;
			}
		}

		for(uint32 i = 0; i < V_COUNTER_COUNT; ++i)
		{
			if(mIntCounter[i])
			{
				mIntCounter[i]->deinit();
				delete mIntCounter[i];
				mIntCounter[i] = nullptr;
			}
		}

		for(uint32 i = 0; i < V_3D_COUNT; ++i)
		{
			if(mInt3D[i])
			{
				mInt3D[i]->deinit();
				delete mInt3D[i];
				mInt3D[i] = nullptr;
			}
		}

		for(const auto& p: mCustom1D)
		{
			p.second->deinit();
			delete p.second;
		}
		mCustom1D.clear();

		for(const auto& p: mCustomCounter)
		{
			p.second->deinit();
			delete p.second;
		}
		mCustomCounter.clear();

		for(const auto& p: mCustom3D)
		{
			p.second->deinit();
			delete p.second;
		}
		mCustom3D.clear();

		for(const auto& p: mCustomSpectral)
		{
			p.second->deinit();
			delete p.second;
		}
		mCustomSpectral.clear();
	}

	void OutputMap::clear()
	{
		mSpectral->clear();

		for(uint32 i = 0; i < V_1D_COUNT; ++i)
		{
			if(mInt1D[i])
				mInt1D[i]->clear();
		}

		for(uint32 i = 0; i < V_COUNTER_COUNT; ++i)
		{
			if(mIntCounter[i])
				mIntCounter[i]->clear();
		}

		for(uint32 i = 0; i < V_3D_COUNT; ++i)
		{
			if(mInt3D[i])
				mInt3D[i]->clear();
		}

		for(const auto& p: mCustom1D)
			p.second->clear();

		for(const auto& p: mCustomCounter)
			p.second->clear();

		for(const auto& p: mCustom3D)
			p.second->clear();

		for(const auto& p: mCustomSpectral)
			p.second->clear();
	}

	void OutputMap::pushFragment(uint32 x, uint32 y, const Spectrum& s, const ShaderClosure& sc)
	{
		uint32 oldSample = getSampleCount(x,y);
		float t = 1.0f/(oldSample + 1.0f);

		PR_CHECK_NEGATIVE(t, "OutputMap::pushFragment");

		// Spectral
		Spectrum oldSpec = mSpectral->getFragmentBounded(x,y);
		Spectrum newSpec = oldSpec * (1-t) + s*t;
		PR_CHECK_NEGATIVE(newSpec, "OutputMap::pushFragment");

		mSpectral->setFragmentBounded(x, y, newSpec);
		setPixelError(x,y,oldSample+1,oldSpec,newSpec);
		setSampleCount(x,y,oldSample+1);

		// 3D
		PM::vec tv = PM::pm_FillVector(t);
		if(mInt3D[V_Position])
			mInt3D[V_Position]->setFragmentBounded(x, y, PM::pm_Lerp(mInt3D[V_Position]->getFragmentBounded(x,y), sc.P, tv));
		if(mInt3D[V_Normal])
			mInt3D[V_Normal]->setFragmentBounded(x, y, PM::pm_Lerp(mInt3D[V_Normal]->getFragmentBounded(x,y), sc.N, tv));
		if(mInt3D[V_NormalG])
			mInt3D[V_NormalG]->setFragmentBounded(x, y, PM::pm_Lerp(mInt3D[V_NormalG]->getFragmentBounded(x,y), sc.Ng, tv));
		if(mInt3D[V_Tangent])
			mInt3D[V_Tangent]->setFragmentBounded(x, y, PM::pm_Lerp(mInt3D[V_Tangent]->getFragmentBounded(x,y), sc.Nx, tv));
		if(mInt3D[V_Bitangent])
			mInt3D[V_Bitangent]->setFragmentBounded(x, y, PM::pm_Lerp(mInt3D[V_Bitangent]->getFragmentBounded(x,y), sc.Ny, tv));
		if(mInt3D[V_View])
			mInt3D[V_View]->setFragmentBounded(x, y, PM::pm_Lerp(mInt3D[V_View]->getFragmentBounded(x,y), sc.V, tv));
		if(mInt3D[V_UVW])
			mInt3D[V_UVW]->setFragmentBounded(x, y, PM::pm_Lerp(mInt3D[V_UVW]->getFragmentBounded(x,y), sc.UV, tv));
		if(mInt3D[V_DPDU])
			mInt3D[V_DPDU]->setFragmentBounded(x, y, PM::pm_Lerp(mInt3D[V_DPDU]->getFragmentBounded(x,y), sc.dPdU, tv));
		if(mInt3D[V_DPDV])
			mInt3D[V_DPDV]->setFragmentBounded(x, y, PM::pm_Lerp(mInt3D[V_DPDV]->getFragmentBounded(x,y), sc.dPdV, tv));
		//if(mInt3D[V_DPDW])
		//	mInt3D[V_DPDW]->setFragmentBounded(x, y, PM::pm_Lerp(mInt3D[V_DPDW]->getFragmentBounded(x,y), sc.dPdW, tv));
		if(mInt3D[V_DPDX])
			mInt3D[V_DPDX]->setFragmentBounded(x, y, PM::pm_Lerp(mInt3D[V_DPDX]->getFragmentBounded(x,y), sc.dPdX, tv));
		if(mInt3D[V_DPDY])
			mInt3D[V_DPDY]->setFragmentBounded(x, y, PM::pm_Lerp(mInt3D[V_DPDY]->getFragmentBounded(x,y), sc.dPdY, tv));
		if(mInt3D[V_DPDZ])
			mInt3D[V_DPDZ]->setFragmentBounded(x, y, PM::pm_Lerp(mInt3D[V_DPDZ]->getFragmentBounded(x,y), sc.dPdZ, tv));
		if(mInt3D[V_DPDT])
			mInt3D[V_DPDT]->setFragmentBounded(x, y, PM::pm_Lerp(mInt3D[V_DPDT]->getFragment(x,y), sc.dPdT, tv));

		// 1D
		if(mInt1D[V_Depth])
			mInt1D[V_Depth]->setFragmentBounded(x, y, mInt1D[V_Depth]->getFragmentBounded(x,y)*(1-t) + std::sqrt(sc.Depth2)*t);
		if(mInt1D[V_Time])
			mInt1D[V_Time]->setFragmentBounded(x, y, mInt1D[V_Time]->getFragmentBounded(x,y)*(1-t) + sc.T*t);
		if(mInt1D[V_Material])
			mInt1D[V_Material]->setFragmentBounded(x, y, mInt1D[V_Material]->getFragmentBounded(x,y)*(1-t) +
				(sc.Material ? sc.Material->id() : 0)*t);
		
		// Counter
		if(mIntCounter[V_ID])
			mIntCounter[V_ID]->setFragmentBounded(x, y, sc.EntityID*t);
	}

	bool OutputMap::isPixelFinished(uint32 x, uint32 y) const
	{
		if(mInt1D[V_Quality] &&
			getSampleCount(x,y) >= mRenderer->settings().minPixelSampleCount())
			return mInt1D[V_Quality]->getFragmentBounded(x, y) <= mRenderer->settings().maxASError();

		return false;
	}

	uint64 OutputMap::finishedPixelCount() const
	{
		const uint32 rw = mRenderer->width();
		const uint32 rh = mRenderer->height();
		const float minError = mRenderer->settings().maxASError();
		const uint32 minSamples = mRenderer->settings().minPixelSampleCount();
		const uint32 maxSamples = mRenderer->settings().maxPixelSampleCount();
		const bool adaptive = mRenderer->settings().isAdaptiveSampling();

		uint64 pixelsFinished = 0;
		if(adaptive)
		{
			for(uint32 j = 0; j < rh; ++j)
			{
				for(uint32 i = 0; i < rw; ++i)
				{
					if(mIntCounter[V_Samples]->getFragmentBounded(i, j) >= minSamples &&
						mInt1D[V_Quality]->getFragmentBounded(i, j) <= minError)
						++pixelsFinished;
				}
			}
		}
		else
		{
			for(uint32 j = 0; j < rh; ++j)
			{
				for(uint32 i = 0; i < rw; ++i)
				{
					if(mIntCounter[V_Samples]->getFragmentBounded(i, j) >= maxSamples)
						++pixelsFinished;
				}
			}
		}

		return pixelsFinished;
	}
}
