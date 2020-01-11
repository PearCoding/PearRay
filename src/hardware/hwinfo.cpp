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

constexpr uint32_t bit(uint32_t i) { return 1 << i; }

// (1)
// EAX
constexpr uint32_t MMX_F  = bit(23);
constexpr uint32_t SSE_F  = bit(25);
constexpr uint32_t SSE2_F = bit(26);
// ECX
constexpr uint32_t SSE3_F   = bit(0);
constexpr uint32_t SSSE3_F  = bit(9);
constexpr uint32_t FMA_F	= bit(12);
constexpr uint32_t SSE41_F  = bit(19);
constexpr uint32_t SSE42_F  = bit(20);
constexpr uint32_t POPCNT_F = bit(23);
constexpr uint32_t AVX_F	= bit(28);

// (7) (0)
// EBX
constexpr uint32_t HLE_F		= bit(4);
constexpr uint32_t AVX2_F		= bit(5);
constexpr uint32_t RTM_F		= bit(11);
constexpr uint32_t AVX512F_F	= bit(16);
constexpr uint32_t AVX512DQ_F   = bit(17);
constexpr uint32_t AVX512IFMA_F = bit(21);
constexpr uint32_t AVX512PF_F   = bit(26);
constexpr uint32_t AVX512ER_F   = bit(27);
constexpr uint32_t AVX512CD_F   = bit(28);
constexpr uint32_t AVX512BW_F   = bit(30);
constexpr uint32_t AVX512VL_F   = bit(31);
// ECX
constexpr uint32_t AVX512VBMI_F		 = bit(1);
constexpr uint32_t AVX512VBMI2_F	 = bit(6);
constexpr uint32_t AVX512VNNI_F		 = bit(11);
constexpr uint32_t AVX512BITALG_F	= bit(12);
constexpr uint32_t AVX512VPOPCNTDQ_F = bit(14);
// EDX
constexpr uint32_t AVX5124VNNIW_F = bit(2);
constexpr uint32_t AVX5124FMAPS_F = bit(3);

// (7) (1)
constexpr uint32_t AVX512BF16_F = bit(5);

// (0x80000001)
// ECX
constexpr uint32_t FMA4_F = bit(16);

struct RequestedFeatures {
	bool MMX			 = false;
	bool SSE			 = false;
	bool SSE2			 = false;
	bool SSE3			 = false;
	bool SSSE3			 = false;
	bool SSE41			 = false;
	bool SSE42			 = false;
	bool AVX			 = false;
	bool AVX2			 = false;
	bool AVX512F		 = false;
	bool AVX512DQ		 = false;
	bool AVX512IFMA		 = false;
	bool AVX512PF		 = false;
	bool AVX512ER		 = false;
	bool AVX512CD		 = false;
	bool AVX512BW		 = false;
	bool AVX512VL		 = false;
	bool AVX512VBMI		 = false;
	bool AVX512VBMI2	 = false;
	bool AVX512VNNI		 = false;
	bool AVX512BITALG	= false;
	bool AVX512VPOPCNTDQ = false;
	bool AVX5124VNNIW	= false;
	bool AVX5124FMAPS	= false;
	bool AVX512BF16		 = false;

	// Transactional Synchronization
	bool HLE = false;
	bool RTM = false;

	// Other useful features
	bool FMA	= false;
	bool FMA4   = false;
	bool POPCNT = false;
};

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

static void printFeatures(const RequestedFeatures& features)
{
	std::cout << std::boolalpha
			  << "MMX: " << features.MMX << std::endl
			  << "SSE: " << features.SSE << std::endl
			  << "SSE2: " << features.SSE2 << std::endl
			  << "SSE3: " << features.SSE3 << std::endl
			  << "SSSE3: " << features.SSSE3 << std::endl
			  << "SSE4_1: " << features.SSE41 << std::endl
			  << "SSE4_2: " << features.SSE42 << std::endl
			  << "AVX: " << features.AVX << std::endl
			  << "AVX2: " << features.AVX2 << std::endl
			  << "AVX512_F: " << features.AVX512F << std::endl
			  << "AVX512_DQ: " << features.AVX512DQ << std::endl
			  << "AVX512_IFMA: " << features.AVX512IFMA << std::endl
			  << "AVX512_PF: " << features.AVX512PF << std::endl
			  << "AVX512_ER: " << features.AVX512ER << std::endl
			  << "AVX512_CD: " << features.AVX512CD << std::endl
			  << "AVX512_BW: " << features.AVX512BW << std::endl
			  << "AVX512_VL: " << features.AVX512VL << std::endl
			  << "AVX512_VBMI: " << features.AVX512VBMI << std::endl
			  << "AVX512_VBMI2: " << features.AVX512VBMI2 << std::endl
			  << "AVX512_VNNI: " << features.AVX512VNNI << std::endl
			  << "AVX512_BITALG: " << features.AVX512BITALG << std::endl
			  << "AVX512_VPOPCNTDQ: " << features.AVX512VPOPCNTDQ << std::endl
			  << "AVX512_4VNNIW: " << features.AVX5124VNNIW << std::endl
			  << "AVX512_4FMAPS: " << features.AVX5124FMAPS << std::endl
			  << "AVX512_BF16: " << features.AVX512BF16 << std::endl
			  << "HLE: " << features.HLE << std::endl
			  << "RTM: " << features.RTM << std::endl
			  << "FMA: " << features.FMA << std::endl
			  << "FMA4: " << features.FMA4 << std::endl
			  << "POPCNT: " << features.POPCNT << std::endl;
}

static void printHeader(const char* filename, const RequestedFeatures& features)
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

	p("MMX", features.MMX);
	p("SSE", features.SSE);
	p("SSE2", features.SSE2);
	p("SSE3", features.SSE3);
	p("SSSE3", features.SSSE3);
	p("SSE4_1", features.SSE41);
	p("SSE4_2", features.SSE42);
	p("AVX", features.AVX);
	p("AVX2", features.AVX2);
	p("AVX512_F", features.AVX512F);
	p("AVX512_DQ", features.AVX512DQ);
	p("AVX512_IFMA", features.AVX512IFMA);
	p("AVX512_PF", features.AVX512PF);
	p("AVX512_ER", features.AVX512ER);
	p("AVX512_CD", features.AVX512CD);
	p("AVX512_BW", features.AVX512BW);
	p("AVX512_VL", features.AVX512VL);
	p("AVX512_VBMI", features.AVX512VBMI);
	p("AVX512_VBMI2", features.AVX512VBMI2);
	p("AVX512_VNNI", features.AVX512VNNI);
	p("AVX512_BITALG", features.AVX512BITALG);
	p("AVX512_VPOPCNTDQ", features.AVX512VPOPCNTDQ);
	p("AVX512_4VNNIW", features.AVX5124VNNIW);
	p("AVX512_4FMAPS", features.AVX5124FMAPS);
	p("AVX512_BF16", features.AVX512BF16);
	p("HLE", features.HLE);
	p("RTM", features.RTM);
	p("FMA", features.FMA);
	p("FMA4", features.FMA4);
	p("POPCNT", features.POPCNT);
}

static RequestedFeatures detectFeatures()
{
	RequestedFeatures features;

	// Processor Info and Feature Bits
	CPUIDOut c1   = i_cpuid(1);
	features.MMX  = c1.EDX & MMX_F;
	features.SSE  = c1.EDX & SSE_F;
	features.SSE2 = c1.EDX & SSE2_F;

	features.SSE3   = c1.ECX & SSE3_F;
	features.SSSE3  = c1.ECX & SSSE3_F;
	features.SSE41  = c1.ECX & SSE41_F;
	features.SSE42  = c1.ECX & SSE42_F;
	features.AVX	= c1.ECX & AVX_F;
	features.FMA	= c1.ECX & FMA_F;
	features.POPCNT = c1.ECX & POPCNT_F;

	// Extended Features
	CPUIDOut c70		= i_cpuid(7, 0);
	features.AVX2		= c70.EBX & AVX2_F;
	features.AVX512F	= c70.EBX & AVX512F_F;
	features.AVX512DQ   = c70.EBX & AVX512DQ_F;
	features.AVX512IFMA = c70.EBX & AVX512IFMA_F;
	features.AVX512PF   = c70.EBX & AVX512PF_F;
	features.AVX512ER   = c70.EBX & AVX512ER_F;
	features.AVX512CD   = c70.EBX & AVX512CD_F;
	features.AVX512BW   = c70.EBX & AVX512BW_F;
	features.AVX512VL   = c70.EBX & AVX512VL_F;

	features.AVX512VBMI		 = c70.ECX & AVX512VBMI_F;
	features.AVX512VBMI2	 = c70.ECX & AVX512VBMI2_F;
	features.AVX512VNNI		 = c70.ECX & AVX512VNNI_F;
	features.AVX512BITALG	= c70.ECX & AVX512BITALG_F;
	features.AVX512VPOPCNTDQ = c70.ECX & AVX512VPOPCNTDQ_F;

	features.AVX5124VNNIW = c70.EDX & AVX5124VNNIW_F;
	features.AVX5124FMAPS = c70.EDX & AVX5124FMAPS_F;

	features.HLE = c70.EBX & HLE_F;
	features.RTM = c70.EBX & RTM_F;

	// Extended Features 2
	CPUIDOut c71		= i_cpuid(7, 1);
	features.AVX512BF16 = c71.EBX & AVX512BF16_F;

	// Highest extension bit
	uint32_t highestExtF = i_cpuid(0x80000000).EAX;

	if (highestExtF >= 0x80000001) {
		CPUIDOut cE1  = i_cpuid(0x80000001);
		features.FMA4 = cE1.ECX & FMA4_F;
	}
	return features;
}

int main(int argc, char** argv)
{
	RequestedFeatures features = detectFeatures();
	printFeatures(features);
	if (argc >= 3) {
		if (argv[1] == std::string("--header"))
			printHeader(argv[2], features);
		else
			std::cerr << "Unknown command " << argv[1] << ". Expected --header" << std::endl;
	}
	return 0;
}