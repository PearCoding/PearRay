#pragma once

#include "Logger.h"
#include "spectral/Spectrum.h"

#include <sstream>

namespace PR
{
	inline std::string dump_negatives(const Spectrum& spec)
	{
		std::stringstream stream;
		stream << "[";
		for(uint32 i = 0; i < Spectrum::SAMPLING_COUNT; ++i)
		{
			if(spec.value(i) < 0.0f)
			{
				stream << i << ":" << spec.value(i);
				if(i < Spectrum::SAMPLING_COUNT-1)
					stream << ";";
			}
		}
		stream << "]";
		return stream.str();
	}

	template<typename T>
	inline T check_negative(const T& v, const char* msg)
	{
		if(v < (T)0)
		{
			PR_LOGGER.logf(PR::L_Fatal, PR::M_Test,
				"Value negative detected! %s -> Value %f", msg, (float)v);// Better solution?
			std::abort();
		}

		return v;
	}

	template<>
	inline Spectrum check_negative<Spectrum>(const Spectrum& spec, const char* msg)
	{
		if(spec.hasNegative())
		{
			PR_LOGGER.logf(PR::L_Fatal, PR::M_Test,
				"Spectrum negative detected! %s -> Negatives %s", msg, dump_negatives(spec).c_str());
			std::abort();
		}

		return spec;
	}

#ifdef PR_ENABLE_DIAGNOSIS
# define PR_CHECK_NEGATIVE(v, msg) check_negative((v),(msg))
#else
# define PR_CHECK_NEGATIVE(v, msg) (v)
#endif

}