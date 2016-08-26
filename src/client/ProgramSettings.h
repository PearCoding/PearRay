#pragma once

#include "renderer/RenderSettings.h"
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

	bool IsVerbose;
	bool IsQuiet;
	bool ShowProgress;

	DisplayDriverOption DDO;

	// Network
	std::string NetIP;
	PR::uint16 NetPort;

	// Image
	float ImgUpdate;
	std::string ImgExt;

	// Threading
	PR::uint32 ThreadCount;
	PR::uint32 TileXCount;
	PR::uint32 TileYCount;

	// Scene
	std::string SceneName;
	std::string CameraOverride;
	PR::uint32 ResolutionXOverride;
	PR::uint32 ResolutionYOverride;
	float CropMinXOverride;
	float CropMaxXOverride;
	float CropMinYOverride;
	float CropMaxYOverride;

	// Render Settings
	PR::RenderSettings RenderSettings;

	bool parse(int argc, char** argv);
};