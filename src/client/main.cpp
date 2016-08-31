#include "Logger.h"
#include "Environment.h"
#include "SceneLoader.h"

#include "renderer/Renderer.h"
#include "renderer/DisplayBuffer.h"

#include "spectral/ToneMapper.h"

#ifdef PR_WITH_NETWORK
# include "NetworkDisplayDriver.h"
#endif

#include "gpu/GPU.h"

#include "FileLogListener.h"

#include "ProgramSettings.h"

#include <boost/filesystem.hpp>

#include <iostream>
#include <chrono>

namespace bf = boost::filesystem;
namespace sc = std::chrono;

void saveImage(DisplayDriverOption displayMode, PR::IDisplayDriver* display,
	const PR::ToneMapper& toneMapper, const bf::path& directoryPath, const std::string& ext)
{
	switch(displayMode)
	{
	case DDO_Image:
		{
			bf::path imagePath = directoryPath;
			imagePath += "/image." + ext;

			if(!reinterpret_cast<PRU::DisplayBuffer*>(display)->save(toneMapper, imagePath.native()))
				PR_LOGGER.logf(PR::L_Error, PR::M_Network, "Couldn't save image to '%s'.", imagePath.c_str());
		}
		break;
#ifdef PR_WITH_NETWORK
	case DDO_Network:
		break;
#endif
	}
}

int main(int argc, char** argv)
{
	ProgramSettings options;
	if(!options.parse(argc, argv))
		return -1;

	time_t t = time(NULL);
	std::stringstream sstream;
#ifdef PR_DEBUG
	sstream << "prc_" << t << "_d.log";
#else
	sstream << "prc_" << t << ".log";
#endif
	const bf::path logFile = options.OutputDir + "/" + sstream.str();

	PR::FileLogListener fileLogListener;
	fileLogListener.open(logFile.native());
	PR_LOGGER.addListener(&fileLogListener);

	PR_LOGGER.setQuiet(options.IsQuiet);
	PR_LOGGER.setVerbose(options.IsVerbose);

	if(!options.IsQuiet)
		std::cout << PR_NAME_STRING << " " << PR_VERSION_STRING << " (C) "  << PR_VENDOR_STRING << std::endl;

	PRU::SceneLoader loader;
	PRU::Environment* env = loader.loadFromFile(options.InputFile);

	if(!env)
	{
		if(!options.IsQuiet)
			std::cout << "Error while parsing input." << std::endl;
		
		return -2;
	} 

	// Setup renderer
	PR::Renderer* renderer = new PR::Renderer(
		options.ResolutionXOverride > 0 ? options.ResolutionXOverride : env->renderWidth(),
		options.ResolutionYOverride > 0 ? options.ResolutionYOverride : env->renderHeight(),
		env->camera(), env->scene(), true);
	renderer->setSettings(options.RenderSettings);
	renderer->setBackgroundMaterial(env->backgroundMaterial());

	if(options.CropMinXOverride >= 0 &&
		options.CropMinXOverride < options.CropMaxXOverride &&
		options.CropMaxXOverride <= 1 &&
		options.CropMinYOverride >= 0 &&
		options.CropMinYOverride < options.CropMaxYOverride &&
		options.CropMaxYOverride <= 1)
	{
		renderer->settings().setCropMaxX(options.CropMaxXOverride);
		renderer->settings().setCropMinX(options.CropMinXOverride);
		renderer->settings().setCropMaxY(options.CropMaxYOverride);
		renderer->settings().setCropMinY(options.CropMinYOverride);
	}
	else
	{
		renderer->settings().setCropMaxX(env->cropMaxX());
		renderer->settings().setCropMinX(env->cropMinX());
		renderer->settings().setCropMaxY(env->cropMaxY());
		renderer->settings().setCropMinY(env->cropMinY());
	}

	PR::ToneMapper toneMapper(renderer->gpu(), renderer->width()*renderer->height(), true);
	PR::IDisplayDriver* display = nullptr;

#ifdef PR_WITH_NETWORK
	asio::io_service* io_service = nullptr;
#endif
	switch(options.DDO)
	{
	case DDO_Image:
		display = new PRU::DisplayBuffer();
		break;
#ifdef PR_WITH_NETWORK
	case DDO_Network:
		try
		{
			io_service = new asio::io_service();
			display = new PRN::NetworkDisplayDriver(*io_service,
				options.NetIP,
				options.NetPort);
			
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

	env->scene()->onPreRender();// Freeze entities
	env->scene()->buildTree();
	if(options.ShowProgress)
		std::cout << "preprocess" << std::endl;

	renderer->start(display, options.TileXCount, options.TileYCount, options.ThreadCount);

	const PR::uint64 maxSamples =
		renderer->renderWidth()*renderer->renderHeight()*renderer->settings().maxPixelSampleCount();
	auto start = sc::high_resolution_clock::now();
	auto start_io = start;
	auto start_prog = start;
	auto start_img = start;
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
		if(options.ShowProgress && span_prog.count() > 1000)
		{
			const float percent = renderer->samplesRendered() / (float)maxSamples;
			std::cout << percent << std::endl;
			start_prog = end;
		}

  		auto span_img = sc::duration_cast<sc::milliseconds>(end - start_img);
		if(options.DDO == DDO_Image && options.ImgUpdate > 0 &&
			span_img.count() > options.ImgUpdate*1000)
		{
			saveImage(options.DDO, display, toneMapper, options.OutputDir, options.ImgExt);
			start_img = end;
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
		
		if(!options.IsQuiet && options.ShowProgress && span.count() >= 1)
			std::cout << std::endl;
		
		PR_LOGGER.logf(PR::L_Info, PR::M_Scene, "Rendering took %d seconds.", span.count());
	}

	// Save images if needed
	saveImage(options.DDO, display, toneMapper, options.OutputDir, options.ImgExt);

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
