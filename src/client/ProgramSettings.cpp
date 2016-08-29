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
	{"halton", SM_HaltonQMC},
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

constexpr PR::uint32 DEF_THREAD_COUNT = 0;
constexpr PR::uint32 DEF_THREAD_TILE_X = 8;
constexpr PR::uint32 DEF_THREAD_TILE_Y = 8;

po::options_description setup_cmd_options()
{
	po::options_description general_d("General");
	general_d.add_options()
		("help,h", "Produce this help message")
		("quiet,q", "Do not print messages into console")
		("verbose,v", "Print detailed information into log file (and perhabs into console)")
		("progress,p", "Show progress (regardless if quiet or not)")

		("input,i", po::value<std::string>(), "Input file")
		("output,o", po::value<std::string>()->default_value("./scene"), "Output directory")
		("config,C", po::value<std::string>(), "Optional configuration file")
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
		("threads,t", po::value<PR::uint32>()->default_value(DEF_THREAD_COUNT),
			"Amount of threads used for processing. Set 0 for automatic detection.")
		("tile_x", po::value<PR::uint32>()->default_value(DEF_THREAD_TILE_X), 
			"Amount of horizontal tiles used in threading")
		("tile_y", po::value<PR::uint32>()->default_value(DEF_THREAD_TILE_Y), 
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
		("inc", po::value<bool>()->default_value(DefaultRenderSettings.isIncremental()),
			"Render incremental.")
		("debug",
			po::value<EnumOption<DebugMode> >()->default_value(DefaultRenderSettings.debugMode()),
		 	(std::string("Debug Mode [") + EnumOption<DebugMode>::get_names() + "]").c_str())
		("depth|d", po::value<PR::uint32>()->default_value(DefaultRenderSettings.maxRayDepth()),
			"Render incremental.")
	;

	po::options_description pixelsampler_d("Pixel Sampler");
	pixelsampler_d.add_options()
		("ps_mode",
			po::value<EnumOption<SamplerMode> >()->default_value(DefaultRenderSettings.pixelSampler()),
		 	(std::string("Pixel Sampler Mode [") + EnumOption<SamplerMode>::get_names() + "]").c_str())
		("ps_max",
			po::value<PR::uint32>()->default_value(DefaultRenderSettings.maxPixelSampleCount()),
		 	"Maximum pixel sample count")
	;

	po::options_description gi_d("Global Illumination");
	gi_d.add_options()
		("gi_diff_max",
			po::value<PR::uint32>()->default_value(DefaultRenderSettings.maxDiffuseBounces()),
		 	"Maximum diffuse bounces")
		("gi_max",
			po::value<PR::uint32>()->default_value(DefaultRenderSettings.maxLightSamples()),
		 	"Maximum light samples")
		("gi_bi",
			po::value<bool>()->default_value(DefaultRenderSettings.isBiDirect()),
		 	"Use bidirect renderer")
	;

	po::options_description photon_d("Photon Mapping");
	photon_d.add_options()
		("p_count",
			po::value<PR::uint32>()->default_value(DefaultRenderSettings.maxPhotons()),
		 	"Photon count")
		("p_radius",
			po::value<float>()->default_value(DefaultRenderSettings.maxPhotonGatherRadius()),
		 	"Maximum gather radius")
		("p_max",
			po::value<PR::uint32>()->default_value(DefaultRenderSettings.maxPhotonGatherCount()),
		 	"Maximum photons used for extimating radiance")
		("p_diff_max",
			po::value<PR::uint32>()->default_value(DefaultRenderSettings.maxPhotonDiffuseBounces()),
		 	"Maximum diffuse bounces")
		("p_spec_min",
			po::value<PR::uint32>()->default_value(DefaultRenderSettings.minPhotonSpecularBounces()),
		 	"Minimum specular bounces")
		("p_g_mode",
			po::value<EnumOption<PhotonGatheringMode> >()->default_value(DefaultRenderSettings.photonGatheringMode()),
		 	(std::string("Photon Gathering Mode [") + EnumOption<PhotonGatheringMode>::get_names() + "]").c_str())
		("p_squeeze",
			po::value<float>()->default_value(DefaultRenderSettings.photonSqueezeWeight()),
		 	"Squeeze Factor")
	;

	po::options_description all_d("Allowed options");
	all_d.add(general_d);
	all_d.add(image_d);
	all_d.add(network_d);
	all_d.add(thread_d);
	all_d.add(scene_d);
	all_d.add(render_d);
	all_d.add(pixelsampler_d);
	all_d.add(gi_d);
	all_d.add(photon_d);

	return all_d;
}

po::options_description setup_ini_options()
{
	PR::RenderSettings DefaultRenderSettings;

	po::options_description all_d;
	all_d.add_options()
		("threads.count", po::value<PR::uint32>()->default_value(DEF_THREAD_COUNT))
		("threads.tile_x", po::value<PR::uint32>()->default_value(DEF_THREAD_TILE_X))
		("threads.tile_y", po::value<PR::uint32>()->default_value(DEF_THREAD_TILE_Y))
		("scene.name", po::value<std::string>())
		("scene.camera", po::value<std::string>())
		("scene.width", po::value<PR::uint32>())
		("scene.height", po::value<PR::uint32>())
		("scene.crop", fixed_tokens_value<std::vector<float> >(4,4))
		("renderer.incremental", po::value<bool>()->default_value(DefaultRenderSettings.isIncremental()))
		("renderer.debug",
			po::value<EnumOption<DebugMode> >()->default_value(DefaultRenderSettings.debugMode()))
		("renderer.max",
			po::value<PR::uint32>()->default_value(DefaultRenderSettings.maxRayDepth()))
		("pixelsampler.mode",
			po::value<EnumOption<SamplerMode> >()->default_value(DefaultRenderSettings.pixelSampler()))
		("pixelsampler.max",
			po::value<PR::uint32>()->default_value(DefaultRenderSettings.maxPixelSampleCount()))
		("globalillumination.diffuse_bounces",
			po::value<PR::uint32>()->default_value(DefaultRenderSettings.maxDiffuseBounces()))
		("globalillumination.light_samples",
			po::value<PR::uint32>()->default_value(DefaultRenderSettings.maxLightSamples()))
		("globalillumination.bidirect",
			po::value<bool>()->default_value(DefaultRenderSettings.isBiDirect()))
		("photon.count",
			po::value<PR::uint32>()->default_value(DefaultRenderSettings.maxPhotons()))
		("photon.radius",
			po::value<float>()->default_value(DefaultRenderSettings.maxPhotonGatherRadius()))
		("photon.max",
			po::value<PR::uint32>()->default_value(DefaultRenderSettings.maxPhotonGatherCount()))
		("photon.max_diffuse_bounces",
			po::value<PR::uint32>()->default_value(DefaultRenderSettings.maxPhotonDiffuseBounces()))
		("photon.min_specular_bounces",
			po::value<PR::uint32>()->default_value(DefaultRenderSettings.minPhotonSpecularBounces()))
		("photon.gathering_mode",
			po::value<EnumOption<PhotonGatheringMode> >()->default_value(DefaultRenderSettings.photonGatheringMode()),
		 	(std::string("Photon Gathering Mode [") + EnumOption<PhotonGatheringMode>::get_names() + "]").c_str())
		("photon.squeeze",
			po::value<float>()->default_value(DefaultRenderSettings.photonSqueezeWeight()))
	;

	return all_d;
}

bool ProgramSettings::parse(int argc, char** argv)
{
	po::options_description all_d = setup_cmd_options();
	po::positional_options_description p;
	p.add("input", 1).add("output", 2);

	po::variables_map vm;
	try
	{
		po::store(po::command_line_parser(argc, argv).options(all_d).positional(p).run(), vm);
		po::notify(vm);
	}
	catch(const std::exception& e)
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

	// Defaults
	SceneName = "";
	CameraOverride = "";
	ResolutionXOverride = 0;
	ResolutionYOverride = 0;
	CropMinXOverride = -1; CropMaxXOverride = -1;
	CropMinYOverride = -1; CropMaxYOverride = -1;

	// First ini file
	if (vm.count("config"))
	{
		po::options_description ini_d = setup_ini_options();
		po::variables_map ini;
		try
		{
			po::store(po::parse_config_file<char>(vm["config"].as<std::string>().c_str(), ini_d), ini);
			po::notify(ini);
		}
		catch(const std::exception& e)
		{
			std::cout << "Error while parsing ini file: " << e.what() << std::endl;
			return false;
		}

		TileXCount = ini["threads.tile_x"].as<PR::uint32>();
		TileYCount = ini["threads.tile_y"].as<PR::uint32>();
		ThreadCount = ini["threads.count"].as<PR::uint32>();
		
		// Scene
		if(ini.count("scene.name"))
			SceneName = ini["scene.name"].as<std::string>();
		
		if(ini.count("scene.camera"))
			CameraOverride = ini["scene.camera"].as<std::string>();
		
		if(ini.count("scene.width"))
			ResolutionXOverride = ini["scene.width"].as<PR::uint32>();
		
		if(ini.count("scene.height"))
			ResolutionYOverride = ini["scene.height"].as<PR::uint32>();

		if(ini.count("scene.crop"))
		{
			std::vector<float> crop = ini["scene.crop"].as<std::vector<float> >();

			PR_ASSERT(crop.size() == 4);
			CropMinXOverride = crop[0]; CropMaxXOverride = crop[1];
			CropMinYOverride = crop[2]; CropMaxYOverride = crop[3];
		}

		// Renderer
		RenderSettings.setIncremental(ini["renderer.incremental"].as<bool>());
		RenderSettings.setDebugMode(ini["renderer.debug"].as<EnumOption<DebugMode> >());

		// PixelSampler
		RenderSettings.setPixelSampler(ini["pixelsampler.mode"].as<EnumOption<SamplerMode> >());
		RenderSettings.setMaxPixelSampleCount(ini["pixelsampler.max"].as<PR::uint32>());

		// Global Illumination
		RenderSettings.setMaxDiffuseBounces(ini["globalillumination.diffuse_bounces"].as<PR::uint32>());
		RenderSettings.setMaxLightSamples(ini["globalillumination.light_samples"].as<PR::uint32>());
		RenderSettings.enableBiDirect(ini["globalillumination.bidirect"].as<bool>());

		// Photon
		RenderSettings.setMaxPhotons(ini["photon.count"].as<PR::uint32>());
		RenderSettings.setMaxPhotonGatherRadius(ini["photon.radius"].as<float>());
		RenderSettings.setMaxPhotonGatherCount(ini["photon.max"].as<PR::uint32>());
		RenderSettings.setMaxPhotonDiffuseBounces(ini["photon.max_diffuse_bounces"].as<PR::uint32>());
		RenderSettings.setMinPhotonSpecularBounces(ini["photon.min_specular_bounces"].as<PR::uint32>());
		RenderSettings.setPhotonGatheringMode(ini["photon.gathering_mode"].as<EnumOption<PhotonGatheringMode> >());
		RenderSettings.setPhotonSqueezeWeight(ini["photon.squeeze"].as<float>());
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
	
	if(vm.count("camera"))
		CameraOverride = vm["camera"].as<std::string>();
	
	if(vm.count("width"))
		ResolutionXOverride = vm["width"].as<PR::uint32>();
	
	if(vm.count("height"))
		ResolutionYOverride = vm["height"].as<PR::uint32>();

	if(vm.count("crop"))
	{
		std::vector<float> crop = vm["crop"].as<std::vector<float> >();

		PR_ASSERT(crop.size() == 4);
		CropMinXOverride = crop[0]; CropMaxXOverride = crop[1];
		CropMinYOverride = crop[2]; CropMaxYOverride = crop[3];
	}

	// Renderer
	RenderSettings.setIncremental(vm["inc"].as<bool>());
	RenderSettings.setDebugMode(vm["debug"].as<EnumOption<DebugMode> >());

	// PixelSampler
	RenderSettings.setPixelSampler(vm["ps_mode"].as<EnumOption<SamplerMode> >());
	RenderSettings.setMaxPixelSampleCount(vm["ps_max"].as<PR::uint32>());

	// Global Illumination
	RenderSettings.setMaxDiffuseBounces(vm["gi_diff_max"].as<PR::uint32>());
	RenderSettings.setMaxLightSamples(vm["gi_max"].as<PR::uint32>());
	RenderSettings.enableBiDirect(vm["gi_bi"].as<bool>());

	// Photon
	RenderSettings.setMaxPhotons(vm["p_count"].as<PR::uint32>());
	RenderSettings.setMaxPhotonGatherRadius(vm["p_radius"].as<float>());
	RenderSettings.setMaxPhotonGatherCount(vm["p_max"].as<PR::uint32>());
	RenderSettings.setMaxPhotonDiffuseBounces(vm["p_diff_max"].as<PR::uint32>());
	RenderSettings.setMinPhotonSpecularBounces(vm["p_spec_min"].as<PR::uint32>());
	RenderSettings.setPhotonGatheringMode(vm["p_g_mode"].as<EnumOption<PhotonGatheringMode> >());
	RenderSettings.setPhotonSqueezeWeight(vm["p_squeeze"].as<float>());
	
	return true;
}
