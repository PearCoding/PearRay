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
   	typedef po::typed_value< T, charT > base;

   	unsigned mMin, mMax;
public:
   	fixed_tokens_typed_value(T * t, unsigned min, unsigned max ) :
	   base( t ), mMin(min), mMax(max)
	{
    	base::multitoken();
   	}

   	virtual fixed_tokens_typed_value* min_tokens( unsigned min )
	{
       	mMin = min;
       	return this;
   	}

   	inline unsigned min_tokens() const { return mMin; }

   	virtual fixed_tokens_typed_value* max_tokens( unsigned max )
	{
       	mMax = max;
       	return this;
   	}

  	inline unsigned max_tokens() const { return mMax; }

    inline base* zero_tokens() {
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

BEGIN_ENUM_OPTION(DisplayDriverOption)
{
	{"image", DDO_Image},
	{"net", DDO_Network},
	{nullptr, DDO_Image}
};

BEGIN_ENUM_OPTION(SamplerMode)
{
	{"rand", SM_Random},
	{"unif", SM_Uniform},
	{"jitt", SM_Jitter},
	{"mjitt", SM_MultiJitter},
	{"halton", SM_HaltonQMC},
	{nullptr, SM_MultiJitter}
};

BEGIN_ENUM_OPTION(TileMode)
{
	{"linear", TM_Linear},
	{"tile", TM_Tile},
	{"spiral", TM_Spiral},
	{nullptr, TM_Linear}
};

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
	{"uv", DM_UVW},
	{"uvw", DM_UVW},
	{"pdf", DM_PDF},
	{"emission", DM_Emission},
	{"validity", DM_Validity},
	{"f_inside", DM_Flag_Inside},
	{"container", DM_Container_ID},
	{nullptr, DM_None}
};

BEGIN_ENUM_OPTION(IntegratorMode)
{
	{"di", IM_Direct},
	{"bidi", IM_BiDirect},
	{"ppm", IM_PPM},
	{nullptr, IM_BiDirect}
};

BEGIN_ENUM_OPTION(TimeMappingMode)
{
	{"center", TMM_Center},
	{"left", TMM_Left},
	{"right", TMM_Right},
	{nullptr, TMM_Center}
};

BEGIN_ENUM_OPTION(PPMGatheringMode)
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
		("progress,p", po::value<PR::uint32>()->implicit_value(1),
			"Show progress (regardless if quiet or not)")
		("information,I", "Print additional scene information into log file (and perhabs into console)")

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

	po::options_description thread_d("Threading[*]");
	thread_d.add_options()
		("threads,t", po::value<PR::uint32>(),
			"Amount of threads used for processing. Set 0 for automatic detection.")
		("rtx", po::value<PR::uint32>(), 
			"Amount of horizontal tiles used in threading")
		("rty", po::value<PR::uint32>(),
			"Amount of vertical tiles used in threading")
		("rtm", po::value<EnumOption<TileMode> >(),
		 	(std::string("Render Tile Mode [") + EnumOption<TileMode>::get_names() + "]").c_str())

	;

	po::options_description scene_d("Scene[*]");
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
		("itx", po::value<PR::uint32>(), 
			"Amount of horizontal image tiles used in rendering")
		("ity", po::value<PR::uint32>(),
			"Amount of vertical image tiles used in rendering")
	;

	PR::RenderSettings DefaultRenderSettings;

	po::options_description render_d("Render[*]");
	render_d.add_options()
		("debug",
			po::value<EnumOption<DebugMode> >(),
		 	(std::string("Debug Mode [") + EnumOption<DebugMode>::get_names() + "]").c_str())
		("integrator",
			po::value<EnumOption<IntegratorMode> >(),
		 	(std::string("Integrator [") + EnumOption<IntegratorMode>::get_names() + "]").c_str())
		("depth|d", po::value<PR::uint32>(),
			"Render incremental.")
	;

	po::options_description sampler_d("Sampler[*]");
	sampler_d.add_options()
		("s_aa",
			po::value<EnumOption<SamplerMode> >(),
		 	(std::string("AA Sampler Mode [") + EnumOption<SamplerMode>::get_names() + "]").c_str())
		("s_aa_max",
			po::value<PR::uint32>(),
		 	"Maximum AA sample count")
		("s_lens",
			po::value<EnumOption<SamplerMode> >(),
		 	(std::string("Lens Sampler Mode [") + EnumOption<SamplerMode>::get_names() + "]").c_str())
		("s_lens_max",
			po::value<PR::uint32>(),
		 	"Maximum Lens sample count")
		("s_time",
			po::value<EnumOption<SamplerMode> >(),
		 	(std::string("Time Sampler Mode [") + EnumOption<SamplerMode>::get_names() + "]").c_str())
		("s_time_max",
			po::value<PR::uint32>(),
		 	"Maximum Time sample count")
		("s_time_mapping",
			po::value<EnumOption<TimeMappingMode> >(),
		 	(std::string("Time Mapping Mode [") + EnumOption<TimeMappingMode>::get_names() + "]").c_str())
		("s_time_scale",
			po::value<PR::uint32>(),
		 	"Time scale")
		("s_spectral",
			po::value<EnumOption<SamplerMode> >(),
		 	(std::string("Spectral Sampler Mode [") + EnumOption<SamplerMode>::get_names() + "]").c_str())
		("s_spectral_max",
			po::value<PR::uint32>(),
		 	"Maximum Spectral sample count")
	;

	po::options_description gi_d("Global Illumination[*]");
	gi_d.add_options()
		("gi_diff_max",
			po::value<PR::uint32>(),
		 	"Maximum diffuse bounces")
		("gi_max",
			po::value<PR::uint32>(),
		 	"Maximum light samples")
	;

	po::options_description photon_d("Progressive Photon Mapping (PPM)[*]");
	photon_d.add_options()
		("p_count",
			po::value<PR::uint32>(),
		 	"Photons per pass")
		("p_passes",
			po::value<PR::uint32>(),
		 	"Max passes")
		("p_radius",
			po::value<float>(),
		 	"Maximum gather radius")
		("p_max",
			po::value<PR::uint32>(),
		 	"Maximum photons used for estimating radiance")
		("p_g_mode",
			po::value<EnumOption<PPMGatheringMode> >(),
		 	(std::string("Photon Gathering Mode [") + EnumOption<PPMGatheringMode>::get_names() + "]").c_str())
		("p_squeeze",
			po::value<float>(),
		 	"Squeeze Factor")
		("p_ratio",
			po::value<float>(),
		 	"Contract ratio")
		("p_proj",
			po::value<float>(),
		 	"Projection Map weight. 0 disables it.")
		("p_proj_qual",
			po::value<float>(),
		 	"Quality of projection map. 1 means highest quality, but more memory requirement.")
		("p_proj_caustic",
			po::value<float>(),
		 	"Ratio of how much to prefer caustic (S*D paths) over other paths. 0 disables it.")
	;

	po::options_description all_d("Allowed options");
	all_d.add(general_d);
	all_d.add(image_d);
	all_d.add(network_d);
	all_d.add(thread_d);
	all_d.add(scene_d);
	all_d.add(render_d);
	all_d.add(sampler_d);
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
		("threads.tile_mode", po::value<EnumOption<TileMode> >()->default_value(DefaultRenderSettings.tileMode()))
		("scene.name", po::value<std::string>())
		("scene.camera", po::value<std::string>())
		("scene.width", po::value<PR::uint32>())
		("scene.height", po::value<PR::uint32>())
		("scene.crop", fixed_tokens_value<std::vector<float> >(4,4))
		("scene.tile_x", po::value<PR::uint32>())
		("scene.tile_y", po::value<PR::uint32>())
		("renderer.integrator",
			po::value<EnumOption<IntegratorMode> >()->default_value(DefaultRenderSettings.integratorMode()))
		("renderer.debug",
			po::value<EnumOption<DebugMode> >()->default_value(DefaultRenderSettings.debugMode()))
		("renderer.max",
			po::value<PR::uint32>()->default_value(DefaultRenderSettings.maxRayDepth()))
		("sampler.aa_mode",
			po::value<EnumOption<SamplerMode> >()->default_value(DefaultRenderSettings.aaSampler()))
		("sampler.aa_max",
			po::value<PR::uint32>()->default_value(DefaultRenderSettings.maxAASampleCount()))
		("sampler.lens_mode",
			po::value<EnumOption<SamplerMode> >()->default_value(DefaultRenderSettings.lensSampler()))
		("sampler.lens_max",
			po::value<PR::uint32>()->default_value(DefaultRenderSettings.maxLensSampleCount()))
		("sampler.time_mode",
			po::value<EnumOption<SamplerMode> >()->default_value(DefaultRenderSettings.timeSampler()))
		("sampler.time_max",
			po::value<PR::uint32>()->default_value(DefaultRenderSettings.maxTimeSampleCount()))
		("sampler.time_mapping",
			po::value<EnumOption<TimeMappingMode> >()->default_value(DefaultRenderSettings.timeMappingMode()))
		("sampler.time_scale",
			po::value<float>()->default_value(DefaultRenderSettings.timeScale()))
		("sampler.spectral_mode",
			po::value<EnumOption<SamplerMode> >()->default_value(DefaultRenderSettings.spectralSampler()))
		("sampler.spectral_max",
			po::value<PR::uint32>()->default_value(DefaultRenderSettings.maxSpectralSampleCount()))
		("globalillumination.diffuse_bounces",
			po::value<PR::uint32>()->default_value(DefaultRenderSettings.maxDiffuseBounces()))
		("globalillumination.light_samples",
			po::value<PR::uint32>()->default_value(DefaultRenderSettings.maxLightSamples()))
		("ppm.count",
			po::value<PR::uint32>()->default_value(DefaultRenderSettings.ppm().maxPhotonsPerPass()))
		("ppm.passes",
			po::value<PR::uint32>()->default_value(DefaultRenderSettings.ppm().maxPassCount()))
		("ppm.radius",
			po::value<float>()->default_value(DefaultRenderSettings.ppm().maxGatherRadius()))
		("ppm.max",
			po::value<PR::uint32>()->default_value(DefaultRenderSettings.ppm().maxGatherCount()))
		("ppm.gathering_mode",
			po::value<EnumOption<PPMGatheringMode> >()->default_value(DefaultRenderSettings.ppm().gatheringMode()))
		("ppm.squeeze",
			po::value<float>()->default_value(DefaultRenderSettings.ppm().squeezeWeight()))
		("ppm.ratio",
			po::value<float>()->default_value(DefaultRenderSettings.ppm().contractRatio()))
		("ppm.proj",
			po::value<float>()->default_value(DefaultRenderSettings.ppm().projectionMapWeight()))
		("ppm.proj_qual",
			po::value<float>()->default_value(DefaultRenderSettings.ppm().projectionMapQuality()))
		("ppm.proj_caustic",
			po::value<float>()->default_value(DefaultRenderSettings.ppm().projectionMapPreferCaustic()))
	;

	return all_d;
}

constexpr float MinMaxASError = 0.0000001f;

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
		std::cout << "See Wiki for more information:\n  https://github.com/PearCoding/PearRay/wiki/PearRayCLI\n" << std::endl;
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
	ImageTileXCount = 1; ImageTileYCount = 1;

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

		RenderTileXCount = ini["threads.tile_x"].as<PR::uint32>();
		RenderTileYCount = ini["threads.tile_y"].as<PR::uint32>();
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

			if(crop.size() != 4)
			{
				std::cout << "Option crop does not met syntax" << std::endl;
				return false;
			}
			CropMinXOverride = crop[0]; CropMaxXOverride = crop[1];
			CropMinYOverride = crop[2]; CropMaxYOverride = crop[3];
		}

		if(ini.count("scene.tile_x"))
			ImageTileXCount = std::max<uint32>(1, ini["scene.tile_x"].as<PR::uint32>());
		if(ini.count("scene.tile_y"))
			ImageTileYCount = std::max<uint32>(1, ini["scene.tile_y"].as<PR::uint32>());

		RenderSettings.setTileMode(ini["threads.tile_mode"].as<EnumOption<TileMode> >());

		// RenderContext
		RenderSettings.setIntegratorMode(ini["renderer.integrator"].as<EnumOption<IntegratorMode> >());
		RenderSettings.setDebugMode(ini["renderer.debug"].as<EnumOption<DebugMode> >());

		// Sampler
		RenderSettings.setAASampler(ini["sampler.aa_mode"].as<EnumOption<SamplerMode> >());
		RenderSettings.setMaxAASampleCount(ini["sampler.aa_max"].as<PR::uint32>());
		RenderSettings.setLensSampler(ini["sampler.lens_mode"].as<EnumOption<SamplerMode> >());
		RenderSettings.setMaxLensSampleCount(ini["sampler.lens_max"].as<PR::uint32>());
		RenderSettings.setTimeSampler(ini["sampler.time_mode"].as<EnumOption<SamplerMode> >());
		RenderSettings.setMaxTimeSampleCount(ini["sampler.time_max"].as<PR::uint32>());
		RenderSettings.setTimeMappingMode(ini["sampler.time_mapping"].as<EnumOption<TimeMappingMode> >());
		RenderSettings.setTimeScale(ini["sampler.time_scale"].as<float>());
		RenderSettings.setSpectralSampler(ini["sampler.spectral_mode"].as<EnumOption<SamplerMode> >());
		RenderSettings.setMaxSpectralSampleCount(ini["sampler.spectral_max"].as<PR::uint32>());
	
		// Global Illumination
		RenderSettings.setMaxDiffuseBounces(ini["globalillumination.diffuse_bounces"].as<PR::uint32>());
		RenderSettings.setMaxLightSamples(ini["globalillumination.light_samples"].as<PR::uint32>());

		// PPM
		RenderSettings.ppm().setMaxPhotonsPerPass(ini["ppm.count"].as<PR::uint32>());
		RenderSettings.ppm().setMaxPassCount(ini["ppm.passes"].as<PR::uint32>());
		RenderSettings.ppm().setMaxGatherRadius(ini["ppm.radius"].as<float>());
		RenderSettings.ppm().setMaxGatherCount(ini["ppm.max"].as<PR::uint32>());
		RenderSettings.ppm().setGatheringMode(ini["ppm.gathering_mode"].as<EnumOption<PPMGatheringMode> >());
		RenderSettings.ppm().setSqueezeWeight(std::min<float>(std::max<float>(ini["ppm.squeeze"].as<float>(), 0), 1));
		RenderSettings.ppm().setContractRatio(std::min<float>(std::max<float>(ini["ppm.ratio"].as<float>(), 0.01f), 1));
		RenderSettings.ppm().setProjectionMapWeight(std::min<float>(std::max<float>(ini["ppm.proj"].as<float>(), 0), 1));
		RenderSettings.ppm().setProjectionMapQuality(std::min<float>(std::max<float>(ini["ppm.proj_qual"].as<float>(), 0.01f), 1));
		RenderSettings.ppm().setProjectionMapPreferCaustic(std::max<float>(ini["ppm.proj_caustic"].as<float>(), 0));
	}

	if(!vm.count("input"))
	{
		std::cout << "No input given!" << std::endl;
		return false;
	}

	// Input file
	InputFile = vm["input"].as<std::string>();
	if(!bf::exists(InputFile))
	{
		std::cout << "Couldn't find file '" << InputFile << "'" << std::endl;
		return false;
	}

	if(!vm.count("output"))
	{
		std::cout << "No output given!" << std::endl;
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
	OutputDir = directoryPath.string();
	// Remove trailing slashes
	if(!OutputDir.empty() && OutputDir.back() == '/')
		OutputDir.pop_back();

	IsVerbose = (vm.count("verbose") != 0);
	IsQuiet = (vm.count("quiet") != 0);
	if(vm.count("progress"))
		ShowProgress = vm["progress"].as<PR::uint32>();
	else
		ShowProgress = 0;
	ShowInformation = (vm.count("information") != 0);

	DDO = vm["display"].as<EnumOption<DisplayDriverOption> >();

	// Network
	NetIP = vm["net-ip"].as<std::string>();
	NetPort = vm["net-port"].as<PR::uint16>();

	// Image
	ImgUpdate = vm["img-update"].as<float>();
	ImgExt = vm["img-ext"].as<std::string>();

	// Thread
	if (vm.count("rtx"))
		RenderTileXCount = vm["rtx"].as<PR::uint32>();
	else if(!vm.count("config"))
		RenderTileXCount = DEF_THREAD_TILE_X;

	if (vm.count("rty"))
		RenderTileYCount = vm["rty"].as<PR::uint32>();
	else if(!vm.count("config"))
		RenderTileYCount = DEF_THREAD_TILE_Y;

	if (vm.count("threads"))
		ThreadCount = vm["threads"].as<PR::uint32>();
	else if(!vm.count("config"))
		ThreadCount = DEF_THREAD_COUNT;
	
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

		if(crop.size() != 4)
		{
			std::cout << "Option crop does not met syntax" << std::endl;
			return false;
		}
		
		CropMinXOverride = crop[0]; CropMaxXOverride = crop[1];
		CropMinYOverride = crop[2]; CropMaxYOverride = crop[3];
	}

	if (vm.count("itx"))
		ImageTileXCount = std::max<uint32>(1, vm["itx"].as<PR::uint32>());
	if (vm.count("ity"))
		ImageTileYCount = std::max<uint32>(1, vm["ity"].as<PR::uint32>());

	if(vm.count("rtm"))
		RenderSettings.setTileMode(vm["rtm"].as<EnumOption<TileMode> >());

	// Renderer
	if(vm.count("integrator"))
		RenderSettings.setIntegratorMode(vm["integrator"].as<EnumOption<IntegratorMode> >());
	if(vm.count("debug"))
		RenderSettings.setDebugMode(vm["debug"].as<EnumOption<DebugMode> >());

	// Sampler
	if(vm.count("s_aa_mode"))
		RenderSettings.setAASampler(vm["s_aa_mode"].as<EnumOption<SamplerMode> >());
	if(vm.count("s_aa_max"))
		RenderSettings.setMaxLensSampleCount(vm["s_aa_max"].as<PR::uint32>());
	if(vm.count("s_lens_mode"))
		RenderSettings.setLensSampler(vm["s_lens_mode"].as<EnumOption<SamplerMode> >());
	if(vm.count("s_lens_max"))
		RenderSettings.setMaxLensSampleCount(vm["s_lens_max"].as<PR::uint32>());
	if(vm.count("s_time_mode"))
		RenderSettings.setTimeSampler(vm["s_time_mode"].as<EnumOption<SamplerMode> >());
	if(vm.count("s_time_max"))
		RenderSettings.setMaxTimeSampleCount(vm["s_time_max"].as<PR::uint32>());
	if(vm.count("s_time_mapping"))
		RenderSettings.setTimeMappingMode(vm["s_time_mapping"].as<EnumOption<TimeMappingMode> >());
	if(vm.count("s_time_scale"))
		RenderSettings.setTimeScale(vm["s_time_scale"].as<float>());
	if(vm.count("s_spectral_mode"))
		RenderSettings.setSpectralSampler(vm["s_spectral_mode"].as<EnumOption<SamplerMode> >());
	if(vm.count("s_spectral_max"))
		RenderSettings.setMaxSpectralSampleCount(vm["s_spectral_max"].as<PR::uint32>());
	
	// Global Illumination
	if(vm.count("gi_diff_max"))
		RenderSettings.setMaxDiffuseBounces(vm["gi_diff_max"].as<PR::uint32>());
	if(vm.count("gi_max"))
		RenderSettings.setMaxLightSamples(vm["gi_max"].as<PR::uint32>());

	// PPM
	if(vm.count("p_count"))
		RenderSettings.ppm().setMaxPhotonsPerPass(vm["p_count"].as<PR::uint32>());
	if(vm.count("p_passes"))
		RenderSettings.ppm().setMaxPassCount(vm["p_passes"].as<PR::uint32>());
	if(vm.count("p_radius"))
		RenderSettings.ppm().setMaxGatherRadius(vm["p_radius"].as<float>());
	if(vm.count("p_max"))
		RenderSettings.ppm().setMaxGatherCount(vm["p_max"].as<PR::uint32>());
	if(vm.count("p_g_mode"))
		RenderSettings.ppm().setGatheringMode(vm["p_g_mode"].as<EnumOption<PPMGatheringMode> >());
	if(vm.count("p_squeeze"))
		RenderSettings.ppm().setSqueezeWeight(std::min<float>(std::max<float>(vm["p_squeeze"].as<float>(), 0), 1));
	if(vm.count("p_ratio"))
		RenderSettings.ppm().setContractRatio(std::min<float>(std::max<float>(vm["p_ratio"].as<float>(), 0.01f), 1));
	if(vm.count("p_proj"))
		RenderSettings.ppm().setProjectionMapWeight(std::min<float>(std::max<float>(vm["p_proj"].as<float>(), 0), 1));
	if(vm.count("p_proj_qual"))
		RenderSettings.ppm().setProjectionMapQuality(std::min<float>(std::max<float>(vm["p_proj_qual"].as<float>(), 0.01f), 1));
	if(vm.count("p_proj_caustic"))
		RenderSettings.ppm().setProjectionMapPreferCaustic(std::max<float>(vm["p_proj_caustic"].as<float>(), 0));

	return true;
}
