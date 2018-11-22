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
	//{"net", DDO_Network},
	{nullptr, DDO_Image}
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
		("registry", "Shows the content of the internal registry")

		("input,i", po::value<std::string>(), "Input file")
		("output,o", po::value<std::string>()->default_value("./scene"), "Output directory")
		("display",
			po::value<EnumOption<DisplayDriverOption> >()->default_value(
				EnumOption<DisplayDriverOption>::get_default()),
		 	(std::string("Display Driver Mode [") + EnumOption<DisplayDriverOption>::get_names() + "]").c_str())
		("pluginpath", po::value<std::string>(), "Additional plugin path")
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
	;

	po::options_description scene_d("Scene[*]");
	scene_d.add_options()
		("itx", po::value<PR::uint32>(), 
			"Amount of horizontal image tiles used in rendering")
		("ity", po::value<PR::uint32>(),
			"Amount of vertical image tiles used in rendering")
	;

	po::options_description all_d("Allowed options");
	all_d.add(general_d);
	all_d.add(image_d);
	all_d.add(network_d);
	all_d.add(thread_d);
	all_d.add(scene_d);

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
		std::cout << "See Wiki for more information:\n  https://github.com/PearCoding/PearRay/wiki/PearRayCLI\n" << std::endl;
		std::cout << all_d << std::endl;
		exit(1);
	}

	// Defaults
	ImageTileXCount = 1; ImageTileYCount = 1;

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

	PluginPath = "";
	if(vm.count("pluginpath"))
	{
		PluginPath = vm["pluginpath"].as<std::string>();

		if(!bf::is_directory(PluginPath)) {
			std::cout << "Given plugin path '" << PluginPath 
				<< "' is not a valid directory" << std::endl;
			return false;
		}
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
	ShowRegistry = (vm.count("registry") != 0);

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

	if (vm.count("itx"))
		ImageTileXCount = std::max<uint32>(1, vm["itx"].as<PR::uint32>());
	if (vm.count("ity"))
		ImageTileYCount = std::max<uint32>(1, vm["ity"].as<PR::uint32>());

	return true;
}
