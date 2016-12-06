#pragma once

#include "Config.h"
#include <string>

#ifdef PR_WITH_GPU
# define __CL_ENABLE_EXCEPTIONS
# include <CL/cl.hpp>
# include <map>
#endif

namespace PR
{
	class PR_LIB GPU
	{
	public:
		GPU();
		~GPU();

		bool init(const std::string& profile);

#ifdef PR_WITH_GPU
		inline cl::Platform& platform()
		{
			return mPlatform;
		}

		inline cl::Context& context()
		{
			return mMainContext;
		}

		inline cl::Device& device()
		{
			return mMainDevice;
		}

		inline cl::Program program(const std::string& name)
		{
			return mPrograms[name];
		}

		static const char* error(cl_int error);
#endif

	private:
#ifdef PR_WITH_GPU
		void addSource(const std::string& name, const std::string& source,
			const std::string& defs);

		cl::Platform mPlatform;
		cl::Context mMainContext;
		cl::Device mMainDevice;

		std::map<std::string, cl::Program> mPrograms;
#endif
	};
}