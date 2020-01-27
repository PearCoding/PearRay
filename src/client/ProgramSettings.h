#pragma once

#include "PR_Config.h"
#include <boost/filesystem.hpp>

class ProgramSettings
{
public:
	boost::filesystem::path InputFile;
	boost::filesystem::path OutputDir;
	boost::filesystem::path PluginPath;

	bool IsVerbose;
	bool IsQuiet;
	PR::uint32 ShowProgress; // In seconds
	bool ShowInformation;

	bool Profile;

	// Image
	PR::uint32 ImgUpdate; // In seconds
	std::string ImgExt;

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