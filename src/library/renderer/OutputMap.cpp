#include "OutputMap.h"
#include "RenderContext.h"
#include "shader/ShaderClosure.h"
#include "spectral/Spectrum.h"
#include "material/Material.h"

#include "Diagnosis.h"

namespace PR
{
	OutputMap::OutputMap(RenderContext* renderer) :
		mRenderer(renderer), mSpectral(new OutputSpectral(renderer))
	{
	}

	OutputMap::~OutputMap()
	{
		deinit();
	}

	void OutputMap::init()
	{
		// Init outputs
		mSpectral->init();
		if(!mIntCounter[V_Samples])
			mIntCounter[V_Samples] = std::make_shared<OutputCounter>(mRenderer, 0);

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
	}

	void OutputMap::deinit()
	{
		mSpectral->deinit();

		for(uint32 i = 0; i < V_1D_COUNT; ++i)
		{
			if(mInt1D[i])
			{
				mInt1D[i]->deinit();
				mInt1D[i].reset();
			}
		}

		for(uint32 i = 0; i < V_COUNTER_COUNT; ++i)
		{
			if(mIntCounter[i])
			{
				mIntCounter[i]->deinit();
				mIntCounter[i].reset();
			}
		}

		for(uint32 i = 0; i < V_3D_COUNT; ++i)
		{
			if(mInt3D[i])
			{
				mInt3D[i]->deinit();
				mInt3D[i].reset();
			}
		}

		for(const auto& p: mCustom1D)
			p.second->deinit();
		mCustom1D.clear();

		for(const auto& p: mCustomCounter)
			p.second->deinit();
		mCustomCounter.clear();

		for(const auto& p: mCustom3D)
			p.second->deinit();
		mCustom3D.clear();

		for(const auto& p: mCustomSpectral)
			p.second->deinit();
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
		setSampleCount(x,y,oldSample+1);

		// 3D
		const PM::vec3 tv = PM::pm_FillVector3D(t);
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
			mInt3D[V_UVW]->setFragmentBounded(x, y, PM::pm_Lerp(mInt3D[V_UVW]->getFragmentBounded(x,y), sc.UVW, tv));
		if(mInt3D[V_DPDU])
			mInt3D[V_DPDU]->setFragmentBounded(x, y, PM::pm_Lerp(mInt3D[V_DPDU]->getFragmentBounded(x,y), sc.dPdU, tv));
		if(mInt3D[V_DPDV])
			mInt3D[V_DPDV]->setFragmentBounded(x, y, PM::pm_Lerp(mInt3D[V_DPDV]->getFragmentBounded(x,y), sc.dPdV, tv));
		if(mInt3D[V_DPDW])
			mInt3D[V_DPDW]->setFragmentBounded(x, y, PM::pm_Lerp(mInt3D[V_DPDW]->getFragmentBounded(x,y), sc.dPdW, tv));
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
		return mIntCounter[V_Samples]->getFragmentBounded(x, y) >= mRenderer->settings().maxCameraSampleCount();
	}

	uint64 OutputMap::finishedPixelCount() const
	{
		const uint32 rw = mRenderer->width();
		const uint32 rh = mRenderer->height();

		uint64 pixelsFinished = 0;
		for(uint32 j = 0; j < rh; ++j)
		{
			for(uint32 i = 0; i < rw; ++i)
			{
				if(isPixelFinished(i,j))
					++pixelsFinished;
			}
		}

		return pixelsFinished;
	}
}
