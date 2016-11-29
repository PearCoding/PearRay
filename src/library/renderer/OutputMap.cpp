#include "OutputMap.h"
#include "Renderer.h"
#include "shader/ShaderClosure.h"
#include "spectral/Spectrum.h"
#include "material/Material.h"

namespace PR
{
	OutputMap::OutputMap(Renderer* renderer) :
		mRenderer(renderer)
	{
		mSpectral = new OutputSpectral(mRenderer);
		std::memset(mInt1D, 0, sizeof(Output1D*)*V_1D_COUNT);
		std::memset(mInt3D, 0, sizeof(Output3D*)*V_3D_COUNT);
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
		if(!mInt1D[V_Samples])
			mInt1D[V_Samples] = new Output1D(mRenderer);

		if(!mInt1D[V_Quality] && mRenderer->settings().isAdaptiveSampling())
			mInt1D[V_Quality] = new Output1D(mRenderer);

		for(uint32 i = 0; i < V_1D_COUNT; ++i)
		{
			if(mInt1D[i])
				mInt1D[i]->init();
		}

		for(uint32 i = 0; i < V_3D_COUNT; ++i)
		{
			if(mInt3D[i])
				mInt3D[i]->init();
		}

		for(Output1D* output: mUser1D)
			output->init();

		for(Output3D* output: mUser3D)
			output->init();

		for(OutputSpectral* output: mUserSpectral)
			output->init();

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

		for(uint32 i = 0; i < V_3D_COUNT; ++i)
		{
			if(mInt3D[i])
			{
				mInt3D[i]->deinit();
				delete mInt3D[i];
				mInt3D[i] = nullptr;
			}
		}

		for(Output1D* output: mUser1D)
		{
			output->deinit();
			delete output;
		}
		mUser1D.clear();

		for(Output3D* output: mUser3D)
		{
			output->deinit();
			delete output;
		}
		mUser3D.clear();

		for(OutputSpectral* output: mUserSpectral)
		{
			output->deinit();
			delete output;
		}
		mUserSpectral.clear();
	}

	void OutputMap::clear()
	{
		mSpectral->clear();

		for(uint32 i = 0; i < V_1D_COUNT; ++i)
		{
			if(mInt1D[i])
				mInt1D[i]->clear();
		}

		for(uint32 i = 0; i < V_3D_COUNT; ++i)
		{
			if(mInt3D[i])
				mInt3D[i]->clear();
		}

		for(Output1D* output: mUser1D)
			output->clear();

		for(Output3D* output: mUser3D)
			output->clear();

		for(OutputSpectral* output: mUserSpectral)
			output->clear();
	}

	void OutputMap::pushFragment(uint32 x, uint32 y, const Spectrum& s, const ShaderClosure& sc)
	{
		uint32 oldSample = getSampleCount(x,y);
		float t = 1.0f/(oldSample + 1.0f);
	
		// Spectral
		Spectrum oldSpec = mSpectral->getFragment(x,y);
		Spectrum newSpec = oldSpec * (1-t) + s*t;

		mSpectral->pushFragment(x, y, newSpec);
		setPixelError(x,y,oldSample+1,oldSpec,newSpec);
		setSampleCount(x,y,oldSample+1);

		// 3D
		PM::vec tv = PM::pm_FillVector(t);
		if(mInt3D[V_Position])
			mInt3D[V_Position]->pushFragment(x, y, PM::pm_Lerp(mInt3D[V_Position]->getFragment(x,y), sc.P, tv));
		if(mInt3D[V_Normal])
			mInt3D[V_Normal]->pushFragment(x, y, PM::pm_Lerp(mInt3D[V_Normal]->getFragment(x,y), sc.N, tv));
		if(mInt3D[V_NormalG])
			mInt3D[V_NormalG]->pushFragment(x, y, PM::pm_Lerp(mInt3D[V_NormalG]->getFragment(x,y), sc.Ng, tv));
		if(mInt3D[V_Tangent])
			mInt3D[V_Tangent]->pushFragment(x, y, PM::pm_Lerp(mInt3D[V_Tangent]->getFragment(x,y), sc.Nx, tv));
		if(mInt3D[V_Bitangent])
			mInt3D[V_Bitangent]->pushFragment(x, y, PM::pm_Lerp(mInt3D[V_Bitangent]->getFragment(x,y), sc.Ny, tv));
		if(mInt3D[V_View])
			mInt3D[V_View]->pushFragment(x, y, PM::pm_Lerp(mInt3D[V_View]->getFragment(x,y), sc.V, tv));
		if(mInt3D[V_UVW])
			mInt3D[V_UVW]->pushFragment(x, y, PM::pm_Lerp(mInt3D[V_UVW]->getFragment(x,y), sc.UV, tv));
		if(mInt3D[V_DPDU])
			mInt3D[V_DPDU]->pushFragment(x, y, PM::pm_Lerp(mInt3D[V_DPDU]->getFragment(x,y), sc.dPdU, tv));
		if(mInt3D[V_DPDV])
			mInt3D[V_DPDV]->pushFragment(x, y, PM::pm_Lerp(mInt3D[V_DPDV]->getFragment(x,y), sc.dPdV, tv));
		//if(mInt3D[V_DPDW])
		//	mInt3D[V_DPDW]->pushFragment(x, y, PM::pm_Lerp(mInt3D[V_DPDW]->getFragment(x,y), sc.dPdW, tv));
		if(mInt3D[V_DPDX])
			mInt3D[V_DPDX]->pushFragment(x, y, PM::pm_Lerp(mInt3D[V_DPDX]->getFragment(x,y), sc.dPdX, tv));
		if(mInt3D[V_DPDY])
			mInt3D[V_DPDY]->pushFragment(x, y, PM::pm_Lerp(mInt3D[V_DPDY]->getFragment(x,y), sc.dPdY, tv));
		if(mInt3D[V_DPDZ])
			mInt3D[V_DPDZ]->pushFragment(x, y, PM::pm_Lerp(mInt3D[V_DPDZ]->getFragment(x,y), sc.dPdZ, tv));
		if(mInt3D[V_DPDT])
			mInt3D[V_DPDT]->pushFragment(x, y, PM::pm_Lerp(mInt3D[V_DPDT]->getFragment(x,y), sc.dPdT, tv));

		// 1D
		if(mInt1D[V_Depth])
			mInt1D[V_Depth]->pushFragment(x, y, mInt1D[V_Depth]->getFragment(x,y)*(1-t) + std::sqrt(sc.Depth2)*t);
		if(mInt1D[V_Time])
			mInt1D[V_Time]->pushFragment(x, y, mInt1D[V_Time]->getFragment(x,y)*(1-t) + sc.T*t);
		if(mInt1D[V_Material])
			mInt1D[V_Material]->pushFragment(x, y, mInt1D[V_Material]->getFragment(x,y)*(1-t) +
				(sc.Material ? sc.Material->id() : 0)*t);
		if(mInt1D[V_ID])
			mInt1D[V_ID]->pushFragment(x, y, mInt1D[V_ID]->getFragment(x,y)*(1-t) + sc.EntityID*t);
	}

	bool OutputMap::isPixelFinished(uint32 x, uint32 y) const
	{
		if(mInt1D[V_Quality] &&
			getSampleCount(x,y) >= mRenderer->settings().minPixelSampleCount())
			return mInt1D[V_Quality]->getFragment(x, y) <= mRenderer->settings().maxASError();

		return false;
	}

	uint64 OutputMap::finishedPixelCount() const
	{
		const uint32 rw = mRenderer->renderWidth();
		const uint32 rh = mRenderer->renderHeight();
		const uint32 sx = mRenderer->cropPixelOffsetX();
		const uint32 sy = mRenderer->cropPixelOffsetY();
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
					if(mInt1D[V_Samples]->getFragment(i+sx, j+sy) >= minSamples &&
						mInt1D[V_Quality]->getFragment(i+sx, j+sy) <= minError)
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
					if(mInt1D[V_Samples]->getFragment(i+sx, j+sy) >= maxSamples)
						++pixelsFinished;
				}
			}
		}

		return pixelsFinished;
	}
}