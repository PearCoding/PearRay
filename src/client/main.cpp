#include "Logger.h"
#include "Environment.h"
#include "SceneLoader.h"

#include "renderer/Renderer.h"
#include "renderer/DisplayBuffer.h"
#include "renderer/IPDisplayDriver.h"

#ifdef PR_WITH_NETWORK
# include "NetworkDisplayDriver.h"
#endif

#include "FileLogListener.h"

#include <boost/program_options/cmdline.hpp>
#include <boost/program_options/options_description.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/program_options/variables_map.hpp>
#include <boost/program_options/positional_options.hpp>

#include <boost/filesystem.hpp>

#include <iostream>
#include <chrono>

namespace po = boost::program_options;
namespace bf = boost::filesystem;
namespace sc = std::chrono;

enum DisplayDriverOption
{
	DDO_Image,
	DDO_IPC,
	DDO_Network
};

std::ostream& operator << (std::ostream& stream, const DisplayDriverOption& ddo)
{
	switch(ddo)
	{
		case DDO_Image:
			stream << "image";
			break;
		case DDO_IPC:
			stream << "ipc";
			break;
		case DDO_Network:
			stream << "net";
			break;
	}
	return stream;
}
void validate(boost::any& v, 
              const std::vector<std::string>& values,
              DisplayDriverOption* target_type, int)
{
    po::validators::check_first_occurrence(v);
    std::string s = po::validators::get_single_string(values);
	std::transform(s.begin(), s.end(), s.begin(), ::tolower);

    if(s == "image") {
        v = boost::any(DDO_Image);
    }
	else if(s == "ipc") {
        v = boost::any(DDO_IPC);
    }
#ifdef PR_WITH_NETWORK
	else if(s == "net") {
        v = boost::any(DDO_Network);
    }
#endif
	else {
        throw po::validation_error(po::validation_error::invalid_option_value);
    }        
}
int main(int argc, char** argv)
{
	po::options_description general_d("General");
	general_d.add_options()
		("help,h", "produce help message")
		("quiet,q", "Do not print messages into console")
		("verbose,v", "Print detailled information into log file (and perhabs into console)")
		("progress,p", "Show progress")

		("input,i", po::value<std::string>(), "input file")
		("output,o", po::value<std::string>()->default_value("./scene"), "output directory")
		("scene", po::value<std::string>(), "a specific scene from file, if not specified, first one will be used")
		("camera", po::value<std::string>(), "if specified, use this camera instead of the camera set in the scene")
		("display", po::value<DisplayDriverOption>()->default_value(DDO_Image), "Display Driver Mode ["
		"Image, IPC"
	#ifdef PR_WITH_NETWORK
		",Net"
	#endif
		"]")
	;

#ifdef PR_WITH_NETWORK
	po::options_description network_d("Network");
	network_d.add_options()
		("net-ip", po::value<std::string>()->default_value("localhost"), "IP address for network interface when network mode is used.")
		("net-port", po::value<short>()->default_value(4242), "Port for network interface when network mode is used.")
	;
#endif

	po::options_description ipc_d("InterProcess");
	ipc_d.add_options()
		("ipc-name", po::value<std::string>()->default_value("pearray_image"), "Name of shared memory region when IPC mode is used. Will be truncated.")
	;

	po::options_description thread_d("Threading");
	thread_d.add_options()
		("threads,t", po::value<unsigned int>()->default_value(0), "Amount of threads used for processing. Set 0 for automatic detection.")
		("tile_x", po::value<unsigned int>()->default_value(8), "Amount of horizontal tiles used in threading")
		("tile_y", po::value<unsigned int>()->default_value(8), "Amount of vertical tiles used in threading")
	;

	po::options_description all_d("Allowed options");
	all_d.add(general_d);
#ifdef PR_WITH_NETWORK
	all_d.add(network_d);
#endif
	all_d.add(ipc_d);
	all_d.add(thread_d);

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
		return -4;
	}    

	// Setup working directory
	const bf::path relativePath = vm["output"].as<std::string>();
	if(!bf::exists(relativePath))
	{
		if(!bf::create_directory(relativePath))
		{
			std::cout << "Coulnd't create directory '" << relativePath << "'" << std::endl;
			return -5;
		}
	}

	const bf::path directoryPath = bf::canonical(relativePath, bf::current_path());
	if(!bf::is_directory(directoryPath))
	{
		std::cout << "Invalid output path given." << std::endl;
		return -6;
	}

	time_t t = time(NULL);
	std::stringstream sstream;
#ifdef PR_DEBUG
	sstream << "prc_" << t << "_d.log";
#else
	sstream << "prc_" << t << ".log";
#endif
	bf::path logFile = directoryPath;
	logFile += "/" + sstream.str();

	PR::FileLogListener fileLogListener;
	fileLogListener.open(logFile.native());
	PR_LOGGER.addListener(&fileLogListener);
	
	const bool beQuiet = (vm.count("quiet") != 0);
	const bool showProgress = (vm.count("progress") != 0);

	if(!beQuiet)
		std::cout << PR_NAME_STRING << " " << PR_VERSION_STRING << " (C) "  << PR_VENDOR_STRING << std::endl;
	else
		PR_LOGGER.setQuiet(true);

	if(vm.count("verbose"))
		PR_LOGGER.setVerbose(true);

	if (vm.count("help"))
	{
		std::cout << all_d << std::endl;
		return 1;
	}

	if (!vm.count("input") || !vm.count("output"))
	{
		if(beQuiet)
			std::cout << "Input and Output have to be set." << std::endl;
		return -1;
	}

	PRU::SceneLoader loader;
	PRU::Environment* env = loader.loadFromFile(vm["input"].as<std::string>());

	if(!env)
	{
		if(beQuiet)
			std::cout << "Error while parsing input." << std::endl;
		
		return -2;
	} 

	// Setup renderer
	PR::Renderer* renderer = new PR::Renderer(env->renderWidth(), env->renderHeight(),
		env->camera(), env->scene());
	renderer->setBackgroundMaterial(env->backgroundMaterial());
	renderer->settings().setCropMaxX(env->cropMaxX());
	renderer->settings().setCropMinX(env->cropMinX());
	renderer->settings().setCropMaxY(env->cropMaxY());
	renderer->settings().setCropMinY(env->cropMinY());

	PR::IDisplayDriver* display = nullptr;
	DisplayDriverOption displayMode = vm["display"].as<DisplayDriverOption>();

#ifdef PR_WITH_NETWORK
	asio::io_service* io_service = nullptr;
#endif
	switch(displayMode)
	{
	case DDO_Image:
		display = new PRU::DisplayBuffer();
		break;
	case DDO_IPC:
		display = new PRU::IPDisplayDriver(vm["ipc-name"].as<std::string>());
		break;
#ifdef PR_WITH_NETWORK
	case DDO_Network:
		try
		{
			io_service = new asio::io_service();
			display = new PRN::NetworkDisplayDriver(*io_service,
				vm["net-ip"].as<std::string>(),
				vm["net-port"].as<short>());
			
			io_service->run();
		}
		catch(std::exception& e)
		{
			PR_LOGGER.logf(PR::L_Fatal, PR::M_Network, "Exception while initializing network: %s", e.what());
			return -3;
		}
		break;
#endif
	}

	env->scene()->buildTree();

	renderer->start(display,
		vm["tile_x"].as<unsigned int>(),
		vm["tile_y"].as<unsigned int>(),
		vm["threads"].as<unsigned int>());

	auto start = sc::high_resolution_clock::now();
	auto start_io = start;
	auto start_prog = start;
	while(!renderer->isFinished())
	{
		auto end = sc::high_resolution_clock::now();

#ifdef PR_WITH_NETWORK
  		auto span_io = sc::duration_cast<sc::milliseconds>(end - start_io);
		if(io_service && span_io.count() > 100)
		{
			PRN::NetworkDisplayDriver* network = reinterpret_cast<PRN::NetworkDisplayDriver*>(display);
			network->handleIO(false);

			start_io = end;
		}
#endif

  		auto span_prog = sc::duration_cast<sc::milliseconds>(end - start_prog);
		if(showProgress && !beQuiet && span_prog.count() > 1000)
		{
			std::cout << "." << std::flush;
			start_prog = end;
		}
	}

#ifdef PR_WITH_NETWORK
	if(io_service)// Send last packets
	{
		reinterpret_cast<PRN::NetworkDisplayDriver*>(display)->handleIO(true);
	}
#endif

	{
		auto end = sc::high_resolution_clock::now();
  		auto span = sc::duration_cast<sc::seconds>(end - start);
		
		if(!beQuiet && showProgress && span.count() >= 1)
			std::cout << std::endl;
		
		PR_LOGGER.logf(PR::L_Info, PR::M_Scene, "Rendering took %d seconds.", span.count());
	}

	// Save images if needed
	switch(displayMode)
	{
	case DDO_Image:
		{
			bf::path imagePath = directoryPath;
			imagePath += "/image.png";

			if(!reinterpret_cast<PRU::DisplayBuffer*>(display)->save(imagePath.native()))
			{
				PR_LOGGER.logf(PR::L_Error, PR::M_Network, "Couldn't save image to '%s'.", imagePath.c_str());
			}
		}
		break;
	case DDO_IPC:
		{
			bf::path imagePath = directoryPath;
			imagePath += "/image.png";

			if(!reinterpret_cast<PRU::IPDisplayDriver*>(display)->save(imagePath.native()))
			{
				PR_LOGGER.logf(PR::L_Error, PR::M_Network, "Couldn't save image to '%s'.", imagePath.c_str());
			}
		}
		break;
#ifdef PR_WITH_NETWORK
	case DDO_Network:
		break;
#endif
	}

	// Close everything
	delete renderer;
	delete display;
	delete env;

#ifdef PR_WITH_NETWORK
	if(io_service)
		delete io_service;
#endif

	return 0;
}
