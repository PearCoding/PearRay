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

#include <OpenImageIO/imageio.h>

namespace po = boost::program_options;
namespace bf = boost::filesystem;

class ProgramSettings {
public:
	bf::path InputFile;
	bf::path ReferenceFile;

	bool IsVerbose;
	bool IsQuiet;

	bool parse(int argc, char** argv);
};

bool ProgramSettings::parse(int argc, char** argv)
{
	po::options_description all_d("Allowed Options");

	// clang-format off
    all_d.add_options()
		("help,h", "Produce this help message")
		("verbose,v", "Print detailed information")
		("input,i", po::value<std::string>(), "Input file")
		("reference,r", po::value<std::string>(), "Reference file")
		;
	// clang-format on

	po::positional_options_description p;
	p.add("input", 1).add("reference", 1);

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
	if (!vm.count("input") || !vm.count("reference")) {
		std::cout << "No input or reference given!" << std::endl;
		return false;
	}

	// Input file
	InputFile = vm["input"].as<std::string>();
	if (!bf::exists(InputFile)) {
		std::cout << "Couldn't find file '" << InputFile << "'" << std::endl;
		return false;
	}

	// Reference file
	ReferenceFile = vm["reference"].as<std::string>();
	if (!bf::exists(ReferenceFile)) {
		std::cout << "Couldn't find file '" << ReferenceFile << "'" << std::endl;
		return false;
	}

	IsVerbose = (vm.count("verbose") != 0);

	return true;
}

bool read_input(const std::string& filename, std::vector<float>& data, int& width, int& height, std::vector<std::string>& channel_names)
{
	auto in = OIIO::ImageInput::open(filename);
	if (!in) {
		std::cerr << "Error: Could not open input file" << std::endl;
		return false;
	}
	const OIIO::ImageSpec& spec = in->spec();
	width						= spec.width;
	height						= spec.height;
	int channels				= spec.nchannels;

	channel_names.resize(channels);
	for (int i = 0; i < channels; ++i)
		channel_names[i] = spec.channel_name(i);

	data.resize(width * height * channels);
	in->read_image(0, channels, OIIO::TypeDesc::FLOAT, &data[0]);
	in->close();

	return true;
}

struct PerChannelStats {
	float Min		 = std::numeric_limits<float>::infinity();
	float MinRef	 = std::numeric_limits<float>::infinity();
	float MinDiff	 = std::numeric_limits<float>::infinity();
	float Max		 = -std::numeric_limits<float>::infinity();
	float MaxRef	 = -std::numeric_limits<float>::infinity();
	float MaxDiff	 = 0;
	float Mean		 = 0;
	float MeanRef	 = 0;
	float MeanDiff	 = 0;
	float MeanSqr	 = 0;
	float MeanSqrRef = 0;

	size_t InfCount = 0;
	size_t NaNCount = 0;

	float MSE  = 0; // = MeanSqrDiff
	float PSNR = 0;

	float SNR	  = 0;
	float SNRRef  = 0;
	float SNRDiff = 0;
	float SNT	  = 0; // Signal to Noise statistics

	float Variance	   = 0;
	float VarianceRef  = 0;
	float VarianceDiff = 0;
};

struct ChannelInfo {
	std::string Name;
	size_t Stride;
	size_t StrideRef;
};

int main(int argc, char** argv)
{
	ProgramSettings options;
	if (!options.parse(argc, argv))
		return EXIT_FAILURE;

	if (options.IsVerbose) {
		std::cout << "Arguments> " << std::endl;
		std::cout << "  Input:     " << options.InputFile << std::endl;
		std::cout << "  Reference: " << options.ReferenceFile << std::endl;
	}

	std::vector<std::string> channel_names1;
	std::vector<float> in_data1;
	int width1 = 0, height1 = 0, channels1 = 0;
	if (!read_input(options.InputFile.generic_string(), in_data1, width1, height1, channel_names1))
		return EXIT_FAILURE;
	channels1 = channel_names1.size();

	std::vector<std::string> channel_names2;
	std::vector<float> in_data2;
	int width2 = 0, height2 = 0, channels2 = 0;
	if (!read_input(options.ReferenceFile.generic_string(), in_data2, width2, height2, channel_names2))
		return EXIT_FAILURE;
	channels2 = channel_names2.size();

	if (width1 != width2 || height1 != height2) {
		std::cerr << "Two inputs does not match in shape" << std::endl;
		return EXIT_FAILURE;
	}

	std::vector<ChannelInfo> channel_info;
	for (size_t i = 0; i < channel_names1.size(); ++i) {
		for (size_t j = 0; j < channel_names2.size(); ++j) {
			if (channel_names1[i] == channel_names2[j]) {
				channel_info.push_back(ChannelInfo{ channel_names1[i], i, j });
				break;
			}
		}
	}

	if (channel_info.empty()) {
		std::cerr << "Error: Could not find the smallest common channels between the given inputs" << std::endl;
		return EXIT_FAILURE;
	}

	size_t channels = channel_info.size();
	std::vector<PerChannelStats> stats(channels);

	// TODO: Median
	const float AVG_FACTOR = 1.0f / (width1 * height1);
	for (size_t k = 0; k < channels; ++k) {
		PerChannelStats& stat = stats[k];
		for (int i = 0; i < width1 * height1; ++i) {
			const float A = in_data1[i * channels1 + channel_info[k].Stride];
			const float B = in_data2[i * channels2 + channel_info[k].StrideRef];

			if (std::isinf(A)) {
				stat.InfCount += 1;
				continue;
			} else if (std::isnan(A)) {
				stat.NaNCount += 1;
				continue;
			}

			stat.Max = std::max(A, stat.Max);
			stat.Min = std::min(A, stat.Min);
			stat.Mean += A;
			stat.MeanSqr += A * A;

			stat.MaxRef = std::max(B, stat.MaxRef);
			stat.MinRef = std::min(B, stat.MinRef);
			stat.MeanRef += B;
			stat.MeanSqrRef += B * B;

			const float diff = std::abs(A - B);
			stat.MaxDiff	 = std::max(diff, stat.MaxDiff);
			stat.MinDiff	 = std::min(diff, stat.MinDiff);
			stat.MeanDiff += diff;
			stat.MSE += diff * diff;
		}

		// Calculate intermediate stats
		stat.Mean *= AVG_FACTOR;
		stat.MeanRef *= AVG_FACTOR;
		stat.MeanDiff *= AVG_FACTOR;
		stat.MeanSqr *= AVG_FACTOR;
		stat.MeanSqrRef *= AVG_FACTOR;
		stat.MSE *= AVG_FACTOR;

		stat.PSNR = 20 * std::log10(stat.MaxRef) - 10 * std::log10(stat.MSE);

		// Calculate variance stats
		for (int i = 0; i < width1 * height1; ++i) {
			const float A = in_data1[i * channels1 + channel_info[k].Stride];
			const float B = in_data2[i * channels2 + channel_info[k].StrideRef];

			if (!std::isfinite(A))
				continue;

			stat.Variance += std::pow(A - stat.Mean, 2);
			stat.VarianceRef += std::pow(B - stat.MeanRef, 2);
			stat.VarianceDiff += std::pow(std::abs(A - B) - stat.MeanDiff, 2);
		}

		stat.Variance *= AVG_FACTOR;
		stat.VarianceRef *= AVG_FACTOR;
		stat.VarianceDiff *= AVG_FACTOR;

		stat.SNR	 = 10 * std::log10(stat.Mean / std::sqrt(stat.Variance));
		stat.SNRRef	 = 10 * std::log10(stat.MeanRef / std::sqrt(stat.VarianceRef));
		stat.SNRDiff = 10 * std::log10(stat.MeanDiff / std::sqrt(stat.VarianceDiff));
		stat.SNT	 = (stat.Mean - stat.MeanRef) / (std::sqrt(stat.Variance) - std::sqrt(stat.VarianceRef));
	}

	// Present statistics
	for (size_t k = 0; k < channels; ++k) {
		const PerChannelStats& stat = stats[k];
		std::cout << "Channel " << channel_info.at(k).Name << ">" << std::endl;
		std::cout << "  -[Min         ] = " << stat.Min << std::endl;
		std::cout << "  -[Max         ] = " << stat.Max << std::endl;
		std::cout << "  -[Mean        ] = " << stat.Mean << std::endl;
		std::cout << "  -[MeanSqr     ] = " << stat.MeanSqr << std::endl;
		std::cout << "  -[MinRef      ] = " << stat.MinRef << std::endl;
		std::cout << "  -[MaxRef      ] = " << stat.MaxRef << std::endl;
		std::cout << "  -[MeanRef     ] = " << stat.MeanRef << std::endl;
		std::cout << "  -[MeanSqrRef  ] = " << stat.MeanSqrRef << std::endl;
		std::cout << "  -[MinDiff     ] = " << stat.MinDiff << std::endl;
		std::cout << "  -[MaxDiff     ] = " << stat.MaxDiff << std::endl;
		std::cout << "  -[MeanDiff    ] = " << stat.MeanDiff << std::endl;
		std::cout << "  -[MSE         ] = " << stat.MSE << std::endl;
		std::cout << "  -[PSNR        ] = " << stat.PSNR << " dB" << std::endl;
		std::cout << "  -[SNR         ] = " << stat.SNR << " dB" << std::endl;
		std::cout << "  -[SNRRef      ] = " << stat.SNRRef << " dB" << std::endl;
		std::cout << "  -[SNRDiff     ] = " << stat.SNRDiff << " dB" << std::endl;
		std::cout << "  -[SNT         ] = " << stat.SNT << std::endl;
		std::cout << "  -[Variance    ] = " << stat.Variance << std::endl;
		std::cout << "  -[VarianceRef ] = " << stat.VarianceRef << std::endl;
		std::cout << "  -[VarianceDiff] = " << stat.VarianceDiff << std::endl;
		std::cout << "  -[StdDev      ] = " << std::sqrt(stat.Variance) << std::endl;
		std::cout << "  -[StdDevRef   ] = " << std::sqrt(stat.VarianceRef) << std::endl;
		std::cout << "  -[StdDevDiff  ] = " << std::sqrt(stat.VarianceDiff) << std::endl;
	}

	return EXIT_SUCCESS;
}
