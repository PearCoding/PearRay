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
	PR::uint32 ShowProgress; // In seconds
	bool ShowInformation;

	bool Profile;

	// Timing
	PR::uint32 MaxTime; // In seconds for equal time measurements
	bool MaxTimeForce;	// Force to stop iteration and do not wait for iteration end

	// Image
	PR::uint32 ImgUpdate; // In seconds
	PR::uint32 ImgUpdateIteration;
	bool ImgUseTags;

	// Network
	PR::int16 ListenNetwork; // Port to listen, -1 no networking

	// Threading
	PR::uint32 ThreadCount;
	bool AdaptiveTiling;
	bool SortHits;
	PR::uint32 RenderTileXCount;
	PR::uint32 RenderTileYCount;
	PR::uint32 ImageTileXCount;
	PR::uint32 ImageTileYCount;

	PR::uint32 CacheMode;

	bool parse(int argc, char** argv);
};
}