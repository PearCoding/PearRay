#include <boost/filesystem.hpp>
#include <boost/program_options/cmdline.hpp>
#include <boost/program_options/options_description.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/program_options/positional_options.hpp>
#include <boost/program_options/variables_map.hpp>
#include <cctype>
#include <cmath>
#include <fstream>
#include <iostream>
#include <sstream>

namespace po = boost::program_options;
namespace bf = boost::filesystem;

constexpr size_t DEF_MIN_SIZE_KB = 1024 * 64; // 64 Mb

class ProgramSettings {
public:
	boost::filesystem::path InputFile;
	boost::filesystem::path OutputDir;

	bool IsVerbose;
	bool IsQuiet;
	bool OverwriteInput;
	bool NoIncludeGenerator;

	size_t MinSizeKb; // In kb

	bool parse(int argc, char** argv);
};

bool ProgramSettings::parse(int argc, char** argv)
{
	po::options_description all_d("Allowed Options");

	// clang-format off
    all_d.add_options()
		("help,h", "Produce this help message")
		("quiet,q", "Do not print messages into console")
		("verbose,v", "Print detailed information")
		("input,i", po::value<std::string>(), "Input file")
		("output,o", po::value<std::string>(), "Output directory")
		("overwrite,f", "Overwrite input file")
		("no-include", "Do not generate file with includes")
		("size,s", po::value<size_t>()->default_value(DEF_MIN_SIZE_KB), "Minimum size of generated chunks. If input is smaller, nothing will be done")
	;
	// clang-format on

	po::positional_options_description p;
	p.add("input", 1).add("output", 2);

	po::variables_map vm;
	try {
		po::store(po::command_line_parser(argc, argv).options(all_d).positional(p).run(), vm);
		po::notify(vm);
	} catch (const std::exception& e) {
		std::cout << "Error while parsing commandline: " << e.what() << std::endl;
		return false;
	}

	// Handle help
	if (vm.count("help")) {
		std::cout << "See Wiki for more information:\n  https://github.com/PearCoding/PearRay/wiki\n"
				  << std::endl;
		std::cout << all_d << std::endl;
		exit(0);
	}

	// Handle version
	if (!vm.count("input")) {
		std::cout << "No input given!" << std::endl;
		return false;
	}

	// Input file
	InputFile = vm["input"].as<std::string>();
	if (!bf::exists(InputFile)) {
		std::cout << "Couldn't find file '" << InputFile << "'" << std::endl;
		return false;
	}

	// Setup output directory
	if (vm.count("output")) {
		const bf::path relativePath = vm["output"].as<std::string>();
		if (!bf::exists(relativePath)) {
			if (!bf::create_directory(relativePath)) {
				std::cout << "Couldn't create directory '" << relativePath << "'" << std::endl;
				return false;
			}
		}

		const bf::path directoryPath = relativePath.is_relative() ? bf::canonical(relativePath, bf::current_path()) : relativePath;
		if (!bf::is_directory(directoryPath)) {
			std::cout << "Invalid output path given." << std::endl;
			return false;
		}
		OutputDir = directoryPath;
	} else {
		OutputDir = bf::path(InputFile).parent_path();
	}

	IsVerbose		   = (vm.count("verbose") != 0);
	IsQuiet			   = (vm.count("quiet") != 0);
	OverwriteInput	 = (vm.count("overwrite") != 0);
	NoIncludeGenerator = (vm.count("no-include") != 0);

	MinSizeKb = vm["size"].as<size_t>();

	return true;
}

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
