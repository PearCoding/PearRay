#pragma once

#include "OutputChannel.h"

#include <list>

namespace PR
{
	class RenderContext;
	struct ShaderClosure;

	class PR_LIB OutputMap
	{
	public:
		enum Variable3D
		{
			V_Position = 0,
			V_Normal,
			V_NormalG,
			V_Tangent,
			V_Bitangent,
			V_View,
			V_UVW,
			V_DPDU,
			V_DPDV,
			V_DPDW,
			V_DPDX,
			V_DPDY,
			V_DPDZ,
			V_DPDT,

			V_3D_COUNT
		};

		enum Variable1D
		{
			V_Depth = 0,
			V_Time,
			V_Quality,
			V_Material,
			V_ID,
			V_Samples,

			V_1D_COUNT
		};

		OutputMap(RenderContext* renderer);
		~OutputMap();
		
		void init();
		void deinit();

		void clear();

		void pushFragment(uint32 x, uint32 y, const Spectrum& s, const ShaderClosure& sc);
		
		inline Spectrum getFragment(uint32 x, uint32 y) const
		{
			return mSpectral->getFragmentBounded(x, y);
		}

		inline void setSampleCount(uint32 x, uint32 y, uint32 sample)
		{
			mInt1D[V_Samples]->pushFragmentBounded(x, y, sample);
		}

		inline uint32 getSampleCount(uint32 x, uint32 y) const
		{
			return mInt1D[V_Samples]->getFragmentBounded(x, y);
		}

		inline void setPixelError(uint32 x, uint32 y, uint32 sample, const Spectrum& pixel, const Spectrum& weight)
		{
			if(!mInt1D[V_Quality])
				return;
			
			// MEAN SQUARE ERROR
			const float t = 1.0f/sample;
			const float s1 = pixel.max();
			const float s2 = weight.max();
			const float ref = (s1 <= PM_EPSILON && s2 <= PM_EPSILON) ? 1 : (s1 + s2);
			const float err = std::abs(s1 - s2) / ref;
			mInt1D[V_Quality]->pushFragmentBounded(x, y,
				mInt1D[V_Quality]->getFragmentBounded(x, y) * (1-t) + t * err * err);	
		}

		bool isPixelFinished(uint32 x, uint32 y) const;
		uint64 finishedPixelCount() const;
	
		inline Output1D* getChannel(Variable1D var) const
		{
			return mInt1D[var];
		}

		inline Output3D* getChannel(Variable3D var) const
		{
			return mInt3D[var];
		}

		inline OutputSpectral* getSpectralChannel() const
		{
			return mSpectral;
		}

		inline void registerChannel(Variable1D var, Output1D* output)
		{
			PR_ASSERT(output);
			PR_ASSERT(!mInt1D[var]);
			mInt1D[var] = output;
		}

		inline void registerChannel(Variable3D var, Output3D* output)
		{
			PR_ASSERT(output);
			PR_ASSERT(!mInt3D[var]);
			mInt3D[var] = output;
		}

		inline void registerUserChannel(Output1D* output)
		{
			PR_ASSERT(output);
			mUser1D.push_back(output);
		}

		inline void registerUserChannel(Output3D* output)
		{
			PR_ASSERT(output);
			mUser3D.push_back(output);
		}

		inline void registerUserChannel(OutputSpectral* output)
		{
			PR_ASSERT(output);
			mUserSpectral.push_back(output);
		}

	private:
		RenderContext* mRenderer;

		OutputSpectral* mSpectral;
		Output3D* mInt3D[V_3D_COUNT];
		Output1D* mInt1D[V_1D_COUNT];
		
		std::list<Output3D*> mUser3D;
		std::list<Output1D*> mUser1D;
		std::list<OutputSpectral*> mUserSpectral;
	};
}