#pragma once

#include "Logger.h"
#include "spectral/Spectrum.h"

#include <sstream>

namespace PR {
inline std::string dump_negatives(const Spectrum& spec)
{
	std::stringstream stream;
	stream << "[";
	for (uint32 i = 0; i < spec.samples(); ++i) {
		if (spec.value(i) < 0.0f) {
			stream << i << ":" << spec.value(i);
			if (i < spec.samples() - 1)
				stream << ";";
		}
	}
	stream << "]";
	return stream.str();
}

template <typename T>
inline T check_negative(const T& v, const char* msg)
{
	if (v < (T)0) {
#ifdef PR_DEBUG
		PR_DEBUG_BREAK();
#endif

		PR_LOGGER.logf(PR::L_Fatal, PR::M_Test,
					   "Value negative detected! %s -> Value %f", msg, (float)v); // Better solution?
		std::abort();
	}

	return v;
}

template <>
inline Spectrum check_negative<Spectrum>(const Spectrum& spec, const char* msg)
{
	if (spec.hasNegative()) {
#ifdef PR_DEBUG
		PR_DEBUG_BREAK();
#endif

		PR_LOGGER.logf(PR::L_Fatal, PR::M_Test,
					   "Spectrum negative detected! %s -> Negatives %s", msg, dump_negatives(spec).c_str());
		std::abort();
	}

	return spec;
}

template <typename T>
inline T check_validity(const T& v, const char* msg)
{
	if (v < (T)0 || !std::isfinite(v)) {
#ifdef PR_DEBUG
		PR_DEBUG_BREAK();
#endif

		PR_LOGGER.logf(PR::L_Fatal, PR::M_Test,
					   "Value is invalid! %s -> Value %f", msg, (float)v); // Better solution?
		std::abort();
	}

	return v;
}

template <>
inline Spectrum check_validity<Spectrum>(const Spectrum& spec, const char* msg)
{
	if (spec.hasNegative() || spec.hasNaN() || spec.hasInf()) {
#ifdef PR_DEBUG
		PR_DEBUG_BREAK();
#endif

		PR_LOGGER.logf(PR::L_Fatal, PR::M_Test,
					   "Spectrum negative detected! %s -> Negatives %s", msg, dump_negatives(spec).c_str());
		std::abort();
	}

	return spec;
}

#ifdef PR_ENABLE_DIAGNOSIS
#define PR_CHECK_NEGATIVE(v, msg) check_negative((v), (msg))
#define PR_CHECK_VALIDITY(v, msg) check_validity((v), (msg))
#else
#define PR_CHECK_NEGATIVE(v, msg) (v)
#define PR_CHECK_VALIDITY(v, msg) (v)
#endif
}
