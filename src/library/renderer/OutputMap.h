#pragma once

#include "OutputChannel.h"

#include <unordered_map>

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

			V_1D_COUNT
		};

		enum VariableCounter
		{
			V_ID = 0,
			V_Samples,

			V_COUNTER_COUNT
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

		inline void setSampleCount(uint32 x, uint32 y, uint64 sample)
		{
			mIntCounter[V_Samples]->setFragmentBounded(x, y, sample);
		}

		inline uint64 getSampleCount(uint32 x, uint32 y) const
		{
			return mIntCounter[V_Samples]->getFragmentBounded(x, y);
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
			mInt1D[V_Quality]->setFragmentBounded(x, y,
				mInt1D[V_Quality]->getFragmentBounded(x, y) * (1-t) + t * err * err);
		}

		bool isPixelFinished(uint32 x, uint32 y) const;
		uint64 finishedPixelCount() const;

		inline Output1D* getChannel(Variable1D var) const
		{
			return mInt1D[var];
		}

		inline OutputCounter* getChannel(VariableCounter var) const
		{
			return mIntCounter[var];
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
			PR_ASSERT(output, "Given output has to be valid");
			PR_ASSERT(!mInt1D[var], "Given output entry has to be set");
			mInt1D[var] = output;
		}

		inline void registerChannel(VariableCounter var, OutputCounter* output)
		{
			PR_ASSERT(output, "Given output has to be valid");
			PR_ASSERT(!mIntCounter[var], "Given output entry has to be set");
			mIntCounter[var] = output;
		}

		inline void registerChannel(Variable3D var, Output3D* output)
		{
			PR_ASSERT(output, "Given output has to be valid");
			PR_ASSERT(!mInt3D[var], "Given output entry has to be set");
			mInt3D[var] = output;
		}

		inline void registerCustomChannel(const std::string& str, Output1D* output)
		{
			PR_ASSERT(output, "Given output has to be valid");
			mCustom1D[str] = output;
		}

		inline void registerCustomChannel(const std::string& str, OutputCounter* output)
		{
			PR_ASSERT(output, "Given output has to be valid");
			mCustomCounter[str] = output;
		}

		inline void registerCustomChannel(const std::string& str, Output3D* output)
		{
			PR_ASSERT(output, "Given output has to be valid");
			mCustom3D[str] = output;
		}

		inline void registerCustomChannel(const std::string& str, OutputSpectral* output)
		{
			PR_ASSERT(output, "Given output has to be valid");
			mCustomSpectral[str] = output;
		}

	private:
		RenderContext* mRenderer;

		OutputSpectral* mSpectral;
		Output3D* mInt3D[V_3D_COUNT];
		Output1D* mInt1D[V_1D_COUNT];
		OutputCounter* mIntCounter[V_COUNTER_COUNT];

		std::unordered_map<std::string, Output3D*> mCustom3D;
		std::unordered_map<std::string, Output1D*> mCustom1D;
		std::unordered_map<std::string, OutputCounter*> mCustomCounter;
		std::unordered_map<std::string, OutputSpectral*> mCustomSpectral;
	};
}
