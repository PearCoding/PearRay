#pragma once

#include "PR_Config.h"

#include <filesystem>

namespace PR {
class ProgramSettings {
public:
	std::filesystem::path InputFile;
	std::filesystem::path OutputDir;
	std::filesystem::path PluginPath;

	bool IsVerbose;
	bool IsQuiet;
	bool NoPrettyConsole;
	uint32 ShowProgress; // In seconds
	bool ShowInformation;

	bool Profile;

	// Timing
	uint32 MaxTime;	   // In seconds for equal time measurements
	bool MaxTimeForce; // Force to stop iteration and do not wait for iteration end

	// Image
	uint32 ImgUpdate; // In seconds
	uint32 ImgUpdateIteration;
	bool ImgUseTags;

	// Tev Image
	uint32 TevUpdate; // In seconds
	uint16 TevPort;
	std::string TevIp;
	bool TevVariance;
	bool TevFeedback;

	// Network
	int16 ListenNetwork; // Port to listen, -1 no networking

	// Threading
	uint32 ThreadCount;
	bool AdaptiveTiling;
	bool SortHits;
	uint32 RenderTileXCount;
	uint32 RenderTileYCount;
	uint32 ImageTileXCount;
	uint32 ImageTileYCount;

	bool Progressive;

	bool parse(int argc, char** argv);
};
} // namespace PR