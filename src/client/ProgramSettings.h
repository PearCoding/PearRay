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
	PR::uint32 ShowProgress;
	bool ShowInformation;
	bool ShowRegistry;

	bool Profile;

	// Image
	float ImgUpdate;
	std::string ImgExt;

	// Threading
	PR::uint32 ThreadCount;
	PR::uint32 RenderTileXCount;
	PR::uint32 RenderTileYCount;
	PR::uint32 ImageTileXCount;
	PR::uint32 ImageTileYCount;

	PR::uint32 CacheMode;
	
	bool parse(int argc, char** argv);
};