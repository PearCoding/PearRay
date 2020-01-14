#pragma once

#include <boost/filesystem.hpp>

class ProgramSettings
{
public:
	boost::filesystem::path InputFile;
	boost::filesystem::path OutputDir;

	bool IsVerbose;
	bool IsQuiet;
	bool OverwriteInput;
	bool NoIncludeGenerator;

    size_t MinSizeKb;// In kb
	
	bool parse(int argc, char** argv);
};