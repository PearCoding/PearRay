#pragma once

#include <string>

class ProgramSettings
{
public:
	std::string Mode;
	bool IsVerbose;
	bool IsQuiet;

	bool parse(int argc, char** argv);
};