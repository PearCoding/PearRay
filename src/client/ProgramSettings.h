#pragma once

#include "PR_Config.h"
#include <string>

enum DisplayDriverOption
{
	DDO_Image,
	DDO_Network
};

class ProgramSettings
{
public:
	std::string InputFile;
	std::string OutputDir;
	std::string PluginPath;

	bool IsVerbose;
	bool IsQuiet;
	PR::uint32 ShowProgress;
	bool ShowInformation;
	bool ShowRegistry;

	DisplayDriverOption DDO;

	// Network
	std::string NetIP;
	PR::uint16 NetPort;

	// Image
	float ImgUpdate;
	std::string ImgExt;

	// Threading
	PR::uint32 ThreadCount;
	PR::uint32 RenderTileXCount;
	PR::uint32 RenderTileYCount;
	PR::uint32 ImageTileXCount;
	PR::uint32 ImageTileYCount;

	bool parse(int argc, char** argv);
};