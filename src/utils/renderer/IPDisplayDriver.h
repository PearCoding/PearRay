#pragma once

#include "renderer/DisplayDriver.h"
#include <string>

#include <boost/interprocess/shared_memory_object.hpp>
#include <boost/interprocess/mapped_region.hpp>

namespace PRU
{
	/* Interprocess Display Driver */
	class PR_LIB_UTILS IPDisplayDriver : public PR::IDisplayDriver
	{
		PR_CLASS_NON_COPYABLE(IPDisplayDriver);
	public:
		IPDisplayDriver(const std::string& name);
		virtual ~IPDisplayDriver();

		void init(PR::Renderer* renderer) override;
		void deinit() override;
		void pushFragment(PR::uint32 x, PR::uint32 y, PR::uint32 layer, PR::uint32 sample, const PR::Spectrum& s) override;
		PR::Spectrum fragment(PR::uint32 x, PR::uint32 y, PR::uint32 layer) const;

		float* ptr() const;
		void clear();

		bool save(const std::string& file) const;
	private:
		std::string mMapName;
		boost::interprocess::shared_memory_object* mSharedMemory;
		boost::interprocess::mapped_region* mMappedRegion;
		PR::Renderer* mRenderer;
	};
}