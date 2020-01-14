#include "ProgramSettings.h"

#include <boost/filesystem.hpp>
#include <cctype>
#include <cmath>
#include <fstream>
#include <iostream>
#include <sstream>

namespace bf = boost::filesystem;

int main(int argc, char** argv)
{
	ProgramSettings options;
	if (!options.parse(argc, argv))
		return -1;

	if (!options.IsQuiet && options.IsVerbose) {
		std::cout << "Arguments> " << options.InputFile << std::endl;
		std::cout << "  Input:             " << options.InputFile << std::endl;
		std::cout << "  Output Dir:        " << options.OutputDir << std::endl;
		std::cout << "  Overwrite:         " << std::boolalpha << options.OverwriteInput << std::endl;
		std::cout << "  Approx Chunk Size: " << options.MinSizeKb << "kB" << std::endl;
	}

	// Check if input is too small
	size_t inputsizekb = bf::file_size(options.InputFile) / 1024;
	if (inputsizekb < options.MinSizeKb) {
		if (!options.IsQuiet)
			std::cout << "Input too small" << std::endl;
		return 0;
	}

	size_t expectedFiles = static_cast<size_t>(std::ceil(inputsizekb / (double)options.MinSizeKb));
	if (!options.IsQuiet && options.IsVerbose)
		std::cout << "Expect " << expectedFiles << " to be generated" << std::endl;

	const std::string basename = options.InputFile.stem().generic_string();
	const std::string ext	  = options.InputFile.extension().generic_string();

	// Init
	auto get_output_path = [&](size_t c) {
		std::stringstream stream;
		stream << basename << "_" << c << ext;
		auto path = options.OutputDir / stream.str();
		return path;
	};
	auto new_output = [&](size_t c) {
		auto path = get_output_path(c);
		if (!options.IsQuiet)
			std::cout << "[" << c << "] " << path << std::endl;
		return std::ofstream(path.string(), std::ios::out);
	};

	std::ifstream input(options.InputFile.string(), std::ios::in);

	size_t readSize				 = 0;
	size_t counter				 = 0;
	size_t level				 = 0;
	std::ofstream current_output = new_output(counter);

	// Start splitting
	while (input) {
		const char c = input.get();
		++readSize;
		if (c == '(') {
			if (level == 0 && readSize > options.MinSizeKb * 1024) { // Split if neccessary
				current_output = new_output(++counter);
				readSize	   = 0;
			}
			current_output.put(c);
			++level;
		} else if (c == ')') {
			current_output.put(c);
			if (level == 0) {
				std::cerr << "Invalid )" << std::endl;
			} else
				--level;
		} else if (c == '"' || c == '\'') { // String
			current_output.put(c);
			const char string_start = c;
			while (input) {
				const char sc = input.get();
				++readSize;
				current_output.put(sc);
				if (sc == '\\' && input) {
					current_output.put(input.get()); // Ignore
					++readSize;
				} else if (sc == string_start)
					break;
			}
		} else if (c == ';') { // Comment
			current_output.put(c);
			while (input) {
				const char cc = input.get();
				++readSize;
				current_output.put(cc);

				if (cc == '\n')
					break;
			}
		} else {
			current_output.put(c);
		}
	}

	input.close();
	current_output.close();

	// Generate include file
	if (!options.NoIncludeGenerator) {
		std::stringstream stream;
		stream << basename << "_inc" << ext;
		auto inc_path	= options.OutputDir / stream.str();
		auto output_path = options.OverwriteInput ? options.InputFile : inc_path;

		std::ofstream output(output_path.string(), std::ios::out);
		for (size_t i = 0; i < counter; ++i) {
			output << "(include '" << get_output_path(i).generic_string() << "')" << std::endl;
		}
	}

	return 0;
}
