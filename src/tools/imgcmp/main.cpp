#include <cmath>
#include <filesystem>
#include <iostream>

#include <cxxopts.hpp>

#include <OpenImageIO/imageio.h>

namespace sf = std::filesystem;

enum CropMode {
	CM_None = 0,
	CM_Pixel,
	CM_Normalized
};

class ProgramSettings {
public:
	sf::path InputFile;
	sf::path ReferenceFile;

	bool IsVerbose;
	bool OnlyColor;
	bool NoGlobalStats;
	std::string Channel;

	::CropMode CropMode;
	float Crop[4];

	bool parse(int argc, char** argv);
};

bool ProgramSettings::parse(int argc, char** argv)
{
	try {
		cxxopts::Options options("prcmp", "Compare two images and calculate multiple statistics");

		// clang-format off
		options.add_options()
			("h,help", "Produce this help message")
			("v,verbose", "Print detailed information")
			("color", "Only check color channels")
			("c,channel", "Check only the given channel", cxxopts::value<std::string>())
			("no-global-stats", "Do not calculate overall statistics by merging all valid channels")
			("i,input", "Input file", cxxopts::value<std::string>())
			("r,reference", "Reference file", cxxopts::value<std::string>())
			("crop", "Only extract information from a region given in pixel coordinates with format sx,sy,ex,ey ", cxxopts::value<std::vector<int>>())
			("ncrop", "Only extract information from a region given in normalized pixel coordinates with format sx,sy,ex,ey ", cxxopts::value<std::vector<float>>())
		;
		// clang-format on
		options.parse_positional({ "input", "reference" });

		auto vm = options.parse(argc, argv);

		// Handle help
		if (vm.count("help")) {
			std::cout << "See Wiki for more information:\n  https://github.com/PearCoding/PearRay/wiki\n"
					  << std::endl;
			std::cout << options.help() << std::endl;
			exit(EXIT_SUCCESS);
		}

		// Handle version
		if (!vm.count("input") || !vm.count("reference")) {
			std::cerr << "Error: No input or reference given!" << std::endl;
			return false;
		}

		// Input file
		InputFile = vm["input"].as<std::string>();
		if (!sf::exists(InputFile)) {
			std::cerr << "Error: Couldn't find file '" << InputFile << "'" << std::endl;
			return false;
		}

		// Reference file
		ReferenceFile = vm["reference"].as<std::string>();
		if (!sf::exists(ReferenceFile)) {
			std::cerr << "Error: Couldn't find file '" << ReferenceFile << "'" << std::endl;
			return false;
		}

		if (vm.count("channel"))
			Channel = vm["channel"].as<std::string>();

		IsVerbose	  = (vm.count("verbose") != 0);
		OnlyColor	  = (vm.count("color") != 0);
		NoGlobalStats = (vm.count("no-global-stats") != 0);

		if (vm.count("crop")) {
			auto& arr = vm["crop"].as<std::vector<int>>();
			if (arr.size() != 4) {
				std::cerr << "Error: ROI has to be defined with two corner points as a list of four integers in format sx,sy,ex,ey" << std::endl;
				return false;
			}

			for (size_t i = 0; i < arr.size(); ++i)
				Crop[i] = arr[i];

			CropMode = CM_Pixel;
		} else if (vm.count("ncrop")) {
			auto& arr = vm["ncrop"].as<std::vector<float>>();
			if (arr.size() != 4) {
				std::cerr << "Error: ROI has to be defined with two corner points as a list of four floats in format sx,sy,ex,ey" << std::endl;
				return false;
			}

			for (size_t i = 0; i < arr.size(); ++i)
				Crop[i] = arr[i];

			CropMode = CM_Normalized;
		} else {
			CropMode = CM_None;
		}
	} catch (const cxxopts::OptionException& e) {
		std::cerr << "Error while parsing commandline: " << e.what() << std::endl;
		return false;
	}

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

static inline float to_db(float f)
{
	return 10 * std::log10(f);
}

struct PerChannelStats {
	size_t N		 = 0;
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
	float MAPE = 0;
};

void mergeStats(PerChannelStats& dst, const PerChannelStats& src)
{
	auto meanAdd = [](float a, size_t n1, float b, size_t n2) {
		return (n1 + n2 == 0) ? 0.0f : (a * n1 + b * n2) / (n1 + n2);
	};

	dst.Min		= std::min(dst.Min, src.Min);
	dst.MinRef	= std::min(dst.MinRef, src.MinRef);
	dst.MinDiff = std::min(dst.MinDiff, src.MinDiff);

	dst.Max		= std::max(dst.Max, src.Max);
	dst.MaxRef	= std::max(dst.MaxRef, src.MaxRef);
	dst.MaxDiff = std::max(dst.MaxDiff, src.MaxDiff);

	dst.Mean	   = meanAdd(dst.Mean, dst.N, src.Mean, src.N);
	dst.MeanRef	   = meanAdd(dst.MeanRef, dst.N, src.MeanRef, src.N);
	dst.MeanDiff   = meanAdd(dst.MeanDiff, dst.N, src.MeanDiff, src.N);
	dst.MeanSqr	   = meanAdd(dst.MeanSqr, dst.N, src.MeanSqr, src.N);
	dst.MeanSqrRef = meanAdd(dst.MeanSqrRef, dst.N, src.MeanSqrRef, src.N);
	dst.MSE		   = meanAdd(dst.MSE, dst.N, src.MSE, src.N);
	dst.MAPE	   = meanAdd(dst.MAPE, dst.N, src.MAPE, src.N);

	dst.InfCount += src.InfCount;
	dst.NaNCount += src.NaNCount;
	dst.N += src.N;
}

void printStat(const PerChannelStats& stat)
{
	// Calculate some final stuff
	float Variance	   = stat.MeanSqr - stat.Mean * stat.Mean;
	float VarianceRef  = stat.MeanSqrRef - stat.MeanRef * stat.MeanRef;
	float VarianceDiff = stat.MSE - stat.MeanDiff * stat.MeanDiff;
	float PSNR		   = stat.MaxRef * stat.MaxRef / stat.MSE;
	float SNR		   = stat.Mean / std::sqrt(Variance);
	float SNRRef	   = stat.MeanRef / std::sqrt(VarianceRef);
	float SNRDiff	   = stat.MeanDiff / std::sqrt(VarianceDiff);
	float SNT		   = (stat.Mean - stat.MeanRef) / (std::sqrt(Variance) - std::sqrt(VarianceRef));

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
	std::cout << "  -[RMSE        ] = " << std::sqrt(stat.MSE) << std::endl;
	std::cout << "  -[MAE         ] = " << stat.MeanDiff << std::endl;
	std::cout << "  -[MAPE        ] = " << stat.MAPE * 100 << " %" << std::endl;
	std::cout << "  -[PSNR        ] = " << PSNR << " [" << to_db(PSNR) << " dB]" << std::endl;
	std::cout << "  -[SNR         ] = " << SNR << " [" << to_db(SNR) << " dB]" << std::endl;
	std::cout << "  -[SNRRef      ] = " << SNRRef << " [" << to_db(SNRRef) << " dB]" << std::endl;
	std::cout << "  -[SNRDiff     ] = " << SNRDiff << " [" << to_db(SNRDiff) << " dB]" << std::endl;
	std::cout << "  -[SNT         ] = " << SNT << std::endl;
	std::cout << "  -[Variance    ] = " << Variance << std::endl;
	std::cout << "  -[VarianceRef ] = " << VarianceRef << std::endl;
	std::cout << "  -[VarianceDiff] = " << VarianceDiff << std::endl;
	std::cout << "  -[StdDev      ] = " << std::sqrt(Variance) << std::endl;
	std::cout << "  -[StdDevRef   ] = " << std::sqrt(VarianceRef) << std::endl;
	std::cout << "  -[StdDevDiff  ] = " << std::sqrt(VarianceDiff) << std::endl;
}

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
		std::cerr << "Error: Two inputs does not match in shape" << std::endl;
		return EXIT_FAILURE;
	}

	std::vector<ChannelInfo> channel_info;
	for (size_t i = 0; i < channel_names1.size(); ++i) {
		if (!options.Channel.empty()) {
			if (channel_names1[i] != options.Channel)
				continue;
		} else {
			if (options.OnlyColor
				&& channel_names1[i] != "R"
				&& channel_names1[i] != "G"
				&& channel_names1[i] != "B")
				continue;
		}

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

	const size_t channels = channel_info.size();
	std::vector<PerChannelStats> stats(channels);

	// Calculate region of interest
	size_t sx = 0;
	size_t sy = 0;
	size_t ex = width1;
	size_t ey = height1;
	if (options.CropMode != CM_None) {
		size_t osx, osy, oex, oey;
		if (options.CropMode == CM_Pixel) {
			osx = (size_t)options.Crop[0];
			osy = (size_t)options.Crop[1];
			oex = (size_t)options.Crop[2];
			oey = (size_t)options.Crop[3];
		} else {
			osx = (size_t)width1 * options.Crop[0];
			osy = (size_t)height1 * options.Crop[1];
			oex = (size_t)width1 * options.Crop[2];
			oey = (size_t)height1 * options.Crop[3];
		}

		sx = std::max(sx, std::min(ex - 1, osx));
		sy = std::max(sy, std::min(ey - 1, osy));
		ex = std::max(sx + 1, std::min(ex, oex));
		ey = std::max(sy + 1, std::min(ey, oey));
	}

	const size_t nw = (ex - sx);
	const size_t nh = (ey - sy);

	// TODO: Median
	const float AVG_FACTOR = 1.0f / (nw * nh);
	for (size_t k = 0; k < channels; ++k) {
		PerChannelStats& stat = stats[k];
		for (size_t y = sy; y < ey; ++y) {
			for (size_t x = sx; x < ex; ++x) {
				const size_t i = y * width1 + x;
				const float A  = in_data1[i * channels1 + channel_info[k].Stride];
				const float B  = in_data2[i * channels2 + channel_info[k].StrideRef];

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
				if (B != 0)
					stat.MAPE += diff / std::abs(B);
			}
		}

		// Calculate intermediate stats
		stat.Mean *= AVG_FACTOR;
		stat.MeanRef *= AVG_FACTOR;
		stat.MeanDiff *= AVG_FACTOR;
		stat.MeanSqr *= AVG_FACTOR;
		stat.MeanSqrRef *= AVG_FACTOR;
		stat.MSE *= AVG_FACTOR;
		stat.MAPE *= AVG_FACTOR;
		stat.N = nw * nh;
	}

	// Present statistics
	for (size_t k = 0; k < channels; ++k) {
		std::cout << "Channel " << channel_info.at(k).Name << ">" << std::endl;
		printStat(stats[k]);
	}

	if (!options.NoGlobalStats && channels > 1) {
		PerChannelStats global;
		for (size_t k = 0; k < channels; ++k)
			mergeStats(global, stats[k]);

		std::cout << "Global>" << std::endl;
		printStat(global);
	}

	return EXIT_SUCCESS;
}
