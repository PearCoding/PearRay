#include <array>
#include <fstream>
#include <iostream>

#if defined(__linux) || defined(linux)
#define _OS_LINUX
#elif defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__WINDOWS__) || defined(__TOS_WIN__)
#define _OS_WINDOWS
#else
#error Your operating system is currently not supported
#endif

#if defined _OS_LINUX
// Nothing
#elif defined _OS_WINDOWS
#include <intrin.h>
#endif

enum CPUID_Mode {
	CM_1_EDX = 0,
	CM_1_ECX,
	CM_7_0_EBX,
	CM_7_0_ECX,
	CM_7_0_EDX,
	CM_7_1_EAX,
	CM_80000001_EDX,
	CM_80000001_ECX,
	_CM_COUNT
};

struct SearchEntry {
	const char* Name;
	CPUID_Mode Mode;
	uint32_t Bit;
};

constexpr uint32_t bit(uint32_t i) { return 1 << i; }

template <typename V, typename... T>
constexpr auto array_of(T&&... t)
	-> std::array<V, sizeof...(T)>
{
	return { { std::forward<T>(t)... } };
}

static const auto Entries = array_of<SearchEntry>(
	SearchEntry{ "MMX", CM_1_EDX, bit(23) },
	SearchEntry{ "SSE", CM_1_EDX, bit(25) },
	SearchEntry{ "SSE2", CM_1_EDX, bit(26) },
	//-------
	SearchEntry{ "SSE3", CM_1_ECX, bit(0) },
	SearchEntry{ "SSSE3", CM_1_ECX, bit(9) },
	SearchEntry{ "FMA", CM_1_ECX, bit(12) },
	SearchEntry{ "SSE4_1", CM_1_ECX, bit(19) },
	SearchEntry{ "SSE4_2", CM_1_ECX, bit(20) },
	SearchEntry{ "POPCNT", CM_1_ECX, bit(23) },
	SearchEntry{ "AVX", CM_1_ECX, bit(28) },
	//-------
	SearchEntry{ "BMI1", CM_7_0_EBX, bit(3) },
	SearchEntry{ "HLE", CM_7_0_EBX, bit(4) },
	SearchEntry{ "AVX2", CM_7_0_EBX, bit(5) },
	SearchEntry{ "BMI2", CM_7_0_EBX, bit(8) },
	SearchEntry{ "RTM", CM_7_0_EBX, bit(11) },
	SearchEntry{ "AVX512F", CM_7_0_EBX, bit(16) },
	SearchEntry{ "AVX512DQ", CM_7_0_EBX, bit(17) },
	SearchEntry{ "AVX512IFMA", CM_7_0_EBX, bit(21) },
	SearchEntry{ "AVX512PF", CM_7_0_EBX, bit(26) },
	SearchEntry{ "AVX512ER", CM_7_0_EBX, bit(27) },
	SearchEntry{ "AVX512CD", CM_7_0_EBX, bit(28) },
	SearchEntry{ "AVX512BW", CM_7_0_EBX, bit(30) },
	SearchEntry{ "AVX512VL", CM_7_0_EBX, bit(31) },
	//-------
	SearchEntry{ "AVX512VBMI", CM_7_0_ECX, bit(1) },
	SearchEntry{ "AVX512VBMI2", CM_7_0_ECX, bit(6) },
	SearchEntry{ "AVX512VNNI", CM_7_0_ECX, bit(11) },
	SearchEntry{ "AVX512BITALG", CM_7_0_ECX, bit(12) },
	SearchEntry{ "AVX512VPOPCNTDQ", CM_7_0_ECX, bit(14) },
	//-------
	SearchEntry{ "AVX5124VNNIW", CM_7_0_EDX, bit(2) },
	SearchEntry{ "AVX5124FMAPS", CM_7_0_EDX, bit(3) },
	//-------
	SearchEntry{ "AVX512BF16", CM_7_1_EAX, bit(5) },
	//-------
	SearchEntry{ "RDTSCP", CM_80000001_EDX, bit(27) },
	//-------
	SearchEntry{ "ABM", CM_80000001_EDX, bit(5) },
	SearchEntry{ "FMA4", CM_80000001_EDX, bit(16) },
	SearchEntry{ "TBM", CM_80000001_EDX, bit(21) });

using ResultVector = std::array<bool, Entries.size()>;

struct CPUIDOut {
	uint32_t EAX;
	uint32_t EBX;
	uint32_t ECX;
	uint32_t EDX;
};

inline CPUIDOut i_cpuid(uint32_t eax)
{
#if defined _OS_LINUX
	CPUIDOut out;
	asm volatile("cpuid"
				 : "=a"(out.EAX),
				   "=b"(out.EBX),
				   "=c"(out.ECX),
				   "=d"(out.EDX)
				 : "0"(eax));
	return out;
#elif defined _OS_WINDOWS
	int cpuinfo[4];
	__cpuid(cpuinfo, eax);
	return CPUIDOut{ (uint32_t)cpuinfo[0], (uint32_t)cpuinfo[1], (uint32_t)cpuinfo[2], (uint32_t)cpuinfo[3] };
#endif
}

inline CPUIDOut i_cpuid(uint32_t eax, uint32_t ecx)
{
#if defined _OS_LINUX
	CPUIDOut out;
	asm volatile("cpuid"
				 : "=a"(out.EAX),
				   "=b"(out.EBX),
				   "=c"(out.ECX),
				   "=d"(out.EDX)
				 : "0"(eax), "2"(ecx));
	return out;
#elif defined _OS_WINDOWS
	int cpuinfo[4];
	__cpuidex(cpuinfo, eax, ecx);
	return CPUIDOut{ (uint32_t)cpuinfo[0], (uint32_t)cpuinfo[1], (uint32_t)cpuinfo[2], (uint32_t)cpuinfo[3] };
#endif
}

static void checkByBits(CPUID_Mode mode, uint32_t bits, ResultVector& results)
{
	for (size_t i = 0; i < Entries.size(); ++i) {
		if (Entries[i].Mode == mode)
			results[i] = Entries[i].Bit & bits;
	}
}

static bool has80000001()
{
	uint32_t highestExtF = i_cpuid(0x80000000).EAX;
	return highestExtF >= 0x80000001;
}

using CheckFunction												 = void (*)(ResultVector& vector);
static const std::array<CheckFunction, _CM_COUNT> CheckFunctions = {
	[](ResultVector& results) { checkByBits(CM_1_EDX, i_cpuid(1).EDX, results); },
	[](ResultVector& results) { checkByBits(CM_1_ECX, i_cpuid(1).ECX, results); },
	[](ResultVector& results) { checkByBits(CM_7_0_EBX, i_cpuid(7, 0).EBX, results); },
	[](ResultVector& results) { checkByBits(CM_7_0_ECX, i_cpuid(7, 0).ECX, results); },
	[](ResultVector& results) { checkByBits(CM_7_0_EDX, i_cpuid(7, 0).EDX, results); },
	[](ResultVector& results) { checkByBits(CM_7_1_EAX, i_cpuid(7, 1).EAX, results); },
	[](ResultVector& results) { if(has80000001()) checkByBits(CM_80000001_ECX, i_cpuid(0x80000001).ECX, results); },
	[](ResultVector& results) { if(has80000001()) checkByBits(CM_80000001_EDX, i_cpuid(0x80000001).EDX, results); }
};

static void printFeatures(const ResultVector& features)
{
	for (size_t i = 0; i < features.size(); ++i)
		std::cout << Entries[i].Name << ": " << std::boolalpha << features[i] << std::endl;
}

static void printHeader(const char* filename, const ResultVector& features)
{
	std::ofstream stream(filename);

	auto p = [&](const char* name, bool b) {
		if (!b)
			stream << "/* #undef PR_HAS_HW_FEATURE_" << name << " */" << std::endl;
		else
			stream << "#define PR_HAS_HW_FEATURE_" << name << std::endl;
	};

	stream << "#pragma once" << std::endl;
	stream << "/* File generated by prhwinfo */" << std::endl
		   << std::endl;

	for (size_t i = 0; i < features.size(); ++i)
		p(Entries[i].Name, features[i]);
}

static ResultVector detectFeatures()
{
	ResultVector features;
	for (int i = 0; i < _CM_COUNT; ++i)
		CheckFunctions[i](features);

	return features;
}

int main(int argc, char** argv)
{
	ResultVector features = detectFeatures();
	printFeatures(features);
	if (argc >= 3) {
		if (argv[1] == std::string("--header"))
			printHeader(argv[2], features);
		else
			std::cerr << "Unknown command " << argv[1] << ". Expected --header" << std::endl;
	}
	return 0;
}