#include "ProgramSettings.h"
#include "EnumOption.h"

#include <boost/program_options/cmdline.hpp>
#include <boost/program_options/options_description.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/program_options/variables_map.hpp>
#include <boost/program_options/positional_options.hpp>

#include <boost/filesystem.hpp>

#include <iostream>

using namespace PR;
namespace po = boost::program_options;
namespace bf = boost::filesystem;

// Utils
// see http://stackoverflow.com/questions/8175723/vector-arguments-in-boost-program-options
template< typename T, typename charT = char >
class fixed_tokens_typed_value : public po::typed_value< T, charT >
{
   	unsigned mMin, mMax;
   	typedef po::typed_value< T, charT > base;
public:
   	fixed_tokens_typed_value(T * t, unsigned min, unsigned max ) :
	   mMin(min), mMax(max), base( t )
	{
    	base::multitoken();
   	}

   	virtual fixed_tokens_typed_value* min_tokens( unsigned min )
	{
       	mMin = min;
       	return this;
   	}

   	unsigned min_tokens() const { return mMin; }

   	virtual fixed_tokens_typed_value* max_tokens( unsigned max )
	{
       	mMax = max;
       	return this;
   	}

  	unsigned max_tokens() const { return mMax; }

   	base* zero_tokens() {
       	mMin = mMax = 0;
       	base::zero_tokens();
       	return *this;
   	}
};

template< typename T >
fixed_tokens_typed_value< T >*
fixed_tokens_value(unsigned min, unsigned max)
{
    return new fixed_tokens_typed_value< T >(nullptr, min, max );
}

template< typename T >
fixed_tokens_typed_value< T >*
fixed_tokens_value(T* t, unsigned min, unsigned max)
{
    return new fixed_tokens_typed_value< T >(t, min, max);
}

template<class T>
void validate(boost::any& v,
              const std::vector<std::string>& values,
              EnumOption<T>* target_type, int)
{
    po::validators::check_first_occurrence(v);
    std::string s = po::validators::get_single_string(values);
	std::transform(s.begin(), s.end(), s.begin(), ::tolower);
	if(EnumOption<T>::has(s))
		v = boost::any(EnumOption<T>(EnumOption<T>::get_value(s)));
	else
        throw po::validation_error(po::validation_error::invalid_option_value);
}

// DisplayDriverOption
BEGIN_ENUM_OPTION(DisplayDriverOption)
{
	{"image", DDO_Image},
	{"net", DDO_Network},
	{nullptr, DDO_Image}
};

// SamplerMode
BEGIN_ENUM_OPTION(SamplerMode)
{
	{"rand", SM_Random},
	{"unif", SM_Uniform},
	{"jitt", SM_Jitter},
	{"mjitt", SM_MultiJitter},
	{nullptr, SM_MultiJitter}
};

// DebugMode
BEGIN_ENUM_OPTION(DebugMode)
{
	{"none", DM_None},
	{"depth", DM_Depth},
	{"norm_b", DM_Normal_Both},
	{"norm_p", DM_Normal_Positive},
	{"norm_n", DM_Normal_Negative},
	{"norm_s", DM_Normal_Spherical},
	{"tang_b", DM_Tangent_Both},
	{"tang_p", DM_Tangent_Positive},
	{"tang_n", DM_Tangent_Negative},
	{"tang_s", DM_Tangent_Spherical},
	{"bino_b", DM_Binormal_Both},
	{"bino_p", DM_Binormal_Positive},
	{"bino_n", DM_Binormal_Negative},
	{"bino_s", DM_Binormal_Spherical},
	{"uv", DM_UV},
	{"pdf", DM_PDF},
	{"applied", DM_Applied},
	{"validity", DM_Validity},
	{nullptr, DM_None}
};
		
// PhotonGatheringMode
BEGIN_ENUM_OPTION(PhotonGatheringMode)
{
	{"dome", PGM_Dome},
	{"sphere", PGM_Sphere},
	{nullptr, PGM_Sphere}
};

po::options_description setup_options()
{
	po::options_description general_d("General");
	general_d.add_options()
		("help,h", "Produce this help message")
		("quiet,q", "Do not print messages into console")
		("verbose,v", "Print detailed information into log file (and perhabs into console)")
		("progress,p", "Show progress (regardless if quiet or not)")

		("input,i", po::value<std::string>(), "Input file")
		("output,o", po::value<std::string>()->default_value("./scene"), "Output directory")
		("display",
			po::value<EnumOption<DisplayDriverOption> >()->default_value(
				EnumOption<DisplayDriverOption>::get_default()),
		 	(std::string("Display Driver Mode [") + EnumOption<DisplayDriverOption>::get_names() + "]").c_str())
	;		

	po::options_description network_d("Network");
	network_d.add_options()
		("net-ip", po::value<std::string>()->default_value("localhost"),
			"IP address for network interface when network mode is used.")
		("net-port", po::value<PR::uint16>()->default_value(4242),
			"Port for network interface when network mode is used.")
	;

	po::options_description image_d("Image");
	image_d.add_options()
		("img-update", po::value<float>()->default_value(0.0f),
			"Update interval in seconds where image will be saved. 0 disables it.")
		("img-ext", po::value<std::string>()->default_value("png"),
			"File extension for image output. Has to be a type supported by OpenImageIO.")
	;

	po::options_description thread_d("Threading");
	thread_d.add_options()
		("threads,t", po::value<PR::uint32>()->default_value(0),
			"Amount of threads used for processing. Set 0 for automatic detection.")
		("tile_x", po::value<PR::uint32>()->default_value(8), 
			"Amount of horizontal tiles used in threading")
		("tile_y", po::value<PR::uint32>()->default_value(8), 
			"Amount of vertical tiles used in threading")
	;

	po::options_description scene_d("Scene");
	scene_d.add_options()
		/*("scene", po::value<std::string>(),
			"If specified, will choose specific scene from input, else, first one will be used")
		("camera", po::value<std::string>(),
			"If specified, will override camera with this value")*/
		("width", po::value<PR::uint32>(), 
			"If specified, will override renderWidth with this value")
		("height", po::value<PR::uint32>(), 
			"If specified, will override renderHeight with this value")
		("crop", fixed_tokens_value<std::vector<float> >(4,4), 
			"(4D Vector) If specified, will override crop with this value")
	;

	PR::RenderSettings DefaultRenderSettings;

	po::options_description render_d("Render");
	render_d.add_options()
		("inc", po::bool_switch()->default_value(DefaultRenderSettings.isIncremental()),
			"Render incremental.")
		("debug",
			po::value<EnumOption<DebugMode> >()->default_value(DefaultRenderSettings.debugMode()),
		 	(std::string("Debug Mode [") + EnumOption<DebugMode>::get_names() + "]").c_str())
		("pixelSampler",
			po::value<EnumOption<SamplerMode> >()->default_value(DefaultRenderSettings.pixelSampler()),
		 	(std::string("Pixel Sampler Mode [") + EnumOption<SamplerMode>::get_names() + "]").c_str())
	;

	po::options_description all_d("Allowed options");
	all_d.add(general_d);
	all_d.add(image_d);
	all_d.add(network_d);
	all_d.add(thread_d);
	all_d.add(scene_d);
	all_d.add(render_d);

	return all_d;
}

bool ProgramSettings::parse(int argc, char** argv)
{
	po::options_description all_d = setup_options();
	po::positional_options_description p;
	p.add("input", 1).add("output", 2);

	po::variables_map vm;
	try
	{
		po::store(po::command_line_parser(argc, argv).options(all_d).positional(p).run(), vm);
		po::notify(vm);
	}
	catch(std::exception& e)
	{
		std::cout << "Error while parsing commandline: " << e.what() << std::endl;
		return false;
	}    

	// Handle help
	if (vm.count("help"))
	{
		std::cout << all_d << std::endl;
		exit(1);
	}

	// Input file
	InputFile = vm["input"].as<std::string>();
	if(!bf::exists(InputFile))
	{
		std::cout << "Couldn't find file '" << InputFile << "'" << std::endl;
		return false;
	}

	// Setup output directory
	const bf::path relativePath = vm["output"].as<std::string>();
	if(!bf::exists(relativePath))
	{
		if(!bf::create_directory(relativePath))
		{
			std::cout << "Couldn't create directory '" << relativePath << "'" << std::endl;
			return false;
		}
	}

	const bf::path directoryPath = relativePath.is_relative() ?
		bf::canonical(relativePath, bf::current_path()) : relativePath;
	if(!bf::is_directory(directoryPath))
	{
		std::cout << "Invalid output path given." << std::endl;
		return false;
	}
	OutputDir = directoryPath.native();

	IsVerbose = (vm.count("verbose") != 0);
	IsQuiet = (vm.count("quiet") != 0);
	ShowProgress = (vm.count("progress") != 0);

	DDO = vm["display"].as<EnumOption<DisplayDriverOption> >();

	// Network
	NetIP = vm["net-ip"].as<std::string>();
	NetPort = vm["net-port"].as<PR::uint16>();

	// Image
	ImgUpdate = vm["img-update"].as<float>();
	ImgExt = vm["img-ext"].as<std::string>();

	// Thread
	TileXCount = vm["tile_x"].as<PR::uint32>();
	TileYCount = vm["tile_y"].as<PR::uint32>();
	ThreadCount = vm["threads"].as<PR::uint32>();
	
	// Scene
	if(vm.count("scene"))
		SceneName = vm["scene"].as<std::string>();
	else
		SceneName = "";
	
	if(vm.count("camera"))
		CameraOverride = vm["camera"].as<std::string>();
	else
		CameraOverride = "";
	
	if(vm.count("width"))
		ResolutionXOverride = vm["width"].as<PR::uint32>();
	else
		ResolutionXOverride = 0;
	
	if(vm.count("height"))
		ResolutionYOverride = vm["height"].as<PR::uint32>();
	else
		ResolutionYOverride = 0;

	if(vm.count("crop"))
	{
		std::vector<float> crop = vm["crop"].as<std::vector<float> >();

		PR_ASSERT(crop.size() == 4);
		CropMinXOverride = crop[0]; CropMaxXOverride = crop[1];
		CropMinYOverride = crop[2]; CropMaxYOverride = crop[3];
	}
	else
	{
		CropMinXOverride = -1; CropMaxXOverride = -1;
		CropMinYOverride = -1; CropMaxYOverride = -1;
	}

	// Renderer
	RenderSettings.setIncremental(vm["inc"].as<bool>());
	RenderSettings.setDebugMode(vm["debug"].as<EnumOption<DebugMode> >());
	RenderSettings.setPixelSampler(vm["pixelSampler"].as<EnumOption<SamplerMode> >());
	
	return true;
}
