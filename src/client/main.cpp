#include "Logger.h"
#include "Environment.h"
#include "SceneLoader.h"

#include "renderer/RenderFactory.h"
#include "renderer/RenderContext.h"
#include "renderer/RenderStatistics.h"

#include "spectral/ToneMapper.h"
#include "spectral/SpectrumDescriptor.h"

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
	std::shared_ptr<PR::Environment> env = PR::SceneLoader::loadFromFile(options.InputFile);
	PR_END_PROFILE_ID(0);

	if(!env)
	{
		if(!options.IsQuiet)
			std::cout << "Error while parsing input." << std::endl;
		
		return -2;
	}

	if(!env->scene().activeCamera())
	{
		if(!options.IsQuiet)
			std::cout << "Error: No camera specified." << std::endl;
		
		return -4;
	}

	// Setup renderFactory
	PR_BEGIN_PROFILE_ID(1);
	PR::RenderFactory* renderFactory = new PR::RenderFactory(
		PR::SpectrumDescriptor::createStandardSpectral(),
		options.ResolutionXOverride > 0 ? options.ResolutionXOverride : env->renderWidth(),
		options.ResolutionYOverride > 0 ? options.ResolutionYOverride : env->renderHeight(),
		env->scene(), options.OutputDir, true);
	renderFactory->setSettings(options.RenderSettings);

	if(options.CropMinXOverride >= 0 &&
		options.CropMinXOverride < options.CropMaxXOverride &&
		options.CropMaxXOverride <= 1 &&
		options.CropMinYOverride >= 0 &&
		options.CropMinYOverride < options.CropMaxYOverride &&
		options.CropMaxYOverride <= 1)
	{
		renderFactory->settings().setCropMaxX(options.CropMaxXOverride);
		renderFactory->settings().setCropMinX(options.CropMinXOverride);
		renderFactory->settings().setCropMaxY(options.CropMaxYOverride);
		renderFactory->settings().setCropMinY(options.CropMinYOverride);
	}
	else
	{
		renderFactory->settings().setCropMaxX(env->cropMaxX());
		renderFactory->settings().setCropMinX(env->cropMinX());
		renderFactory->settings().setCropMaxY(env->cropMaxY());
		renderFactory->settings().setCropMinY(env->cropMinY());
	}

	PR_END_PROFILE_ID(1);

	PR_BEGIN_PROFILE_ID(2);
	env->scene().freeze();// Freeze entities
	PR_END_PROFILE_ID(2);

	PR_BEGIN_PROFILE_ID(3);
	env->scene().buildTree();
	PR_END_PROFILE_ID(3);

	// Render per image tile
	for(PR::uint32 i = 0; i < options.ImageTileXCount * options.ImageTileYCount; ++i)
	{
		auto renderer =
			renderFactory->create(i, options.ImageTileXCount, options.ImageTileYCount);
		if(options.ImageTileXCount * options.ImageTileYCount == 1)
		{
			PR_LOGGER.logf(PR::L_Info, PR::M_Scene, "Starting rendering of image [%i, %i] x [%i, %i]",
				renderer->offsetX(), renderer->offsetX()+renderer->width(),
				renderer->offsetY(), renderer->offsetY()+renderer->height());
		}
		else
		{
			PR_LOGGER.logf(PR::L_Info, PR::M_Scene, "Starting rendering of image tile %i / %i [%i, %i] x [%i, %i]",
				renderer->index() + 1, options.ImageTileXCount * options.ImageTileYCount,
				renderer->offsetX(), renderer->offsetX()+renderer->width(),
				renderer->offsetY(), renderer->offsetY()+renderer->height());
		}

		env->outputSpecification().setup(renderer);
		env->scene().setup(renderer);
			
		if(options.ShowInformation)
			env->dumpInformation();
		
		PR::ToneMapper toneMapper(renderer->width(), renderer->height(), renderFactory->gpu());

		if(options.ShowProgress)
			std::cout << "preprocess" << std::endl;

		renderer->start(options.RenderTileXCount, options.RenderTileYCount, options.ThreadCount);

		auto start = sc::high_resolution_clock::now();
		auto start_prog = start;
		auto start_img = start;
		while(!renderer->isFinished())
		{
			auto end = sc::high_resolution_clock::now();
			auto span_prog = sc::duration_cast<sc::seconds>(end - start_prog);
			if(options.ShowProgress > 0 && span_prog.count() > options.ShowProgress)
			{
				PR::RenderStatus status = renderer->status();

				std::cout << std::setprecision(6) << status.percentage()*100 << "%"
					<< " Pass " << renderer->currentPass() + 1;

				if(status.hasField("int.feedback"))
					std::cout << "( " << status.getField("int.feedback").getString() << ")";

				std::cout << " | S: " << status.getField("global.pixel_sample_count").getUInt() 
					<< " R: " << status.getField("global.ray_count").getUInt()
					<< " EH: " << status.getField("global.entity_hit_count").getUInt()
					<< " BH: " << status.getField("global.background_hit_count").getUInt()
					<< std::endl;
				
				start_prog = end;
			}

			auto span_img = sc::duration_cast<sc::milliseconds>(end - start_img);
			if(options.DDO == DDO_Image && options.ImgUpdate > 0 &&
				span_img.count() > options.ImgUpdate*1000)
			{
				env->save(renderer, toneMapper, false);
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

		// Save images
		env->save(renderer, toneMapper, true);
	}

	env->outputSpecification().deinit();
	delete renderFactory;

#ifdef PR_PROFILE
	std::string prof_file = options.OutputDir + "/profile.out";
	PR::PerformanceWriter::write(prof_file);
#endif

	return 0;
}
