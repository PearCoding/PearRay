#include "Logger.h"
#include "Environment.h"
#include "SceneLoader.h"

#include "renderer/Renderer.h"
#include "renderer/RenderStatistics.h"

#include "spectral/ToneMapper.h"

#include "gpu/GPU.h"

#include "FileLogListener.h"

#include "ProgramSettings.h"

#ifdef PR_PROFILE
# include "performance/PerformanceWriter.h"
#endif
#include "performance/Performance.h"

#include <boost/filesystem.hpp>

#include <iostream>
#include <iomanip>
#include <chrono>

namespace bf = boost::filesystem;
namespace sc = std::chrono;

int main(int argc, char** argv)
{
	ProgramSettings options;
	if(!options.parse(argc, argv))
		return -1;

	time_t t = time(NULL);
	std::stringstream sstream;
#ifdef PR_DEBUG
	sstream << "pr_" << t << "_d.log";
#else
	sstream << "pr_" << t << ".log";
#endif
	const bf::path logFile = options.OutputDir + "/" + sstream.str();

	PR::FileLogListener fileLogListener;
	fileLogListener.open(logFile.native());
	PR_LOGGER.addListener(&fileLogListener);

	PR_LOGGER.setQuiet(options.IsQuiet);
	PR_LOGGER.setVerbose(options.IsVerbose);

	if(!options.IsQuiet)
		std::cout << PR_NAME_STRING << " " << PR_VERSION_STRING << " (C) "  << PR_VENDOR_STRING << std::endl;

	PR_BEGIN_PROFILE_ID(0);
	PRU::SceneLoader loader;
	PRU::Environment* env = loader.loadFromFile(options.InputFile);
	PR_END_PROFILE_ID(0);

	if(!env || !env->scene())
	{
		if(!options.IsQuiet)
			std::cout << "Error while parsing input." << std::endl;
		
		return -2;
	}

	if(!env->camera())
	{
		if(!options.IsQuiet)
			std::cout << "Error: No camera specified." << std::endl;
		
		return -4;
	}

	// Setup renderer
	PR_BEGIN_PROFILE_ID(1);
	PR::Renderer* renderer = new PR::Renderer(
		options.ResolutionXOverride > 0 ? options.ResolutionXOverride : env->renderWidth(),
		options.ResolutionYOverride > 0 ? options.ResolutionYOverride : env->renderHeight(),
		env->camera(), env->scene(), options.OutputDir, true);
	renderer->setSettings(options.RenderSettings);

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

	PR::ToneMapper toneMapper(renderer->gpu(), renderer->width()*renderer->height());
	PR_END_PROFILE_ID(1);

	PR_BEGIN_PROFILE_ID(2);
	env->scene()->freeze();// Freeze entities
	PR_END_PROFILE_ID(2);

	PR_BEGIN_PROFILE_ID(3);
	env->scene()->buildTree();
	PR_END_PROFILE_ID(3);

	env->outputSpecification().init(renderer);
	env->outputSpecification().setup();

	if(options.ShowProgress)
		std::cout << "preprocess" << std::endl;

	renderer->start(options.TileXCount, options.TileYCount, options.ThreadCount);

	auto start = sc::high_resolution_clock::now();
	auto start_prog = start;
	auto start_img = start;
	while(!renderer->isFinished())
	{
		auto end = sc::high_resolution_clock::now();
  		auto span_prog = sc::duration_cast<sc::seconds>(end - start_prog);
		if(options.ShowProgress > 0 && span_prog.count() > options.ShowProgress)
		{
			PR::RenderStatistics stats = renderer->stats();

			std::cout << std::setprecision(6) << renderer->percentFinished() << "%"
				<< " Pass " << renderer->currentPass() + 1 
				<< " | S: " << stats.pixelSampleCount() 
				<< " R: " << stats.rayCount() 
				<< " EH: " << stats.entityHitCount() 
				<< " BH: " << stats.backgroundHitCount() << std::endl;
			start_prog = end;
		}

  		auto span_img = sc::duration_cast<sc::milliseconds>(end - start_img);
		if(options.DDO == DDO_Image && options.ImgUpdate > 0 &&
			span_img.count() > options.ImgUpdate*1000)
		{
			env->outputSpecification().save(toneMapper, false);
			start_img = end;
		}
	}

	{
		auto end = sc::high_resolution_clock::now();
  		auto span = sc::duration_cast<sc::seconds>(end - start);
		
		if(!options.IsQuiet && options.ShowProgress && span.count() >= 1)
			std::cout << std::endl;
		
		PR_LOGGER.logf(PR::L_Info, PR::M_Scene, "Rendering took %d seconds.", span.count());
	}

	// Save images if needed
	env->outputSpecification().save(toneMapper, true);
	env->outputSpecification().deinit();

	// Close everything
	delete renderer;
	delete env;

#ifdef PR_PROFILE
	std::string prof_file = options.OutputDir + "/profile.out";
	PR::PerformanceWriter::write(prof_file);
#endif

	return 0;
}
