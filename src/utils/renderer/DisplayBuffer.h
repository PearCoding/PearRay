#pragma once

#include "renderer/DisplayDriver.h"
#include <string>

namespace PRU
{
	class PR_LIB_UTILS DisplayBuffer : public PR::IDisplayDriver
	{
		PR_CLASS_NON_COPYABLE(DisplayBuffer);
	public:
		DisplayBuffer();
		virtual ~DisplayBuffer();

		void init(PR::Renderer* renderer) override;
		void deinit() override;
		void pushFragment(PR::uint32 x, PR::uint32 y, PR::uint32 layer, PR::uint32 sample, const PR::Spectrum& s) override;
		PR::Spectrum fragment(PR::uint32 x, PR::uint32 y, PR::uint32 layer) const;

		float* ptr() const;
		void clear();

		bool save(const std::string& file) const;
	private:
		float* mData;
		PR::Renderer* mRenderer;
	};
}