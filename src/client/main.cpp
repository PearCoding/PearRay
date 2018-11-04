#include "Environment.h"
#include "Logger.h"
#include "SceneLoader.h"

#include "renderer/RenderContext.h"
#include "renderer/RenderFactory.h"
#include "renderer/RenderStatistics.h"

#include "spectral/SpectrumDescriptor.h"
#include "spectral/ToneMapper.h"

#include "FileLogListener.h"

#include "ProgramSettings.h"

#include <boost/filesystem.hpp>

#include <chrono>
#include <iomanip>
#include <iostream>

namespace bf = boost::filesystem;
namespace sc = std::chrono;

constexpr int OUTPUT_FIELD_SIZE = 8;
int main(int argc, char** argv)
{
	ProgramSettings options;
	if (!options.parse(argc, argv))
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
	PR_LOGGER.setVerbosity(options.IsVerbose ? PR::L_DEBUG : PR::L_INFO);

	if (!options.IsQuiet)
		std::cout << PR_NAME_STRING << " " << PR_VERSION_STRING << " (C) " << PR_VENDOR_STRING << std::endl;

	std::shared_ptr<PR::Environment> env = PR::SceneLoader::loadFromFile(
		options.OutputDir, options.InputFile, options.PluginPath);

	if (!env) {
		if (!options.IsQuiet)
			std::cout << "Error while parsing input." << std::endl;

		return -2;
	}

	// Setup renderFactory
	auto renderFactory = env->renderManager().createRenderFactory();
	if (!renderFactory) {
		if (!options.IsQuiet)
			std::cout << "Error: Couldn't setup render factory." << std::endl;

		return -4;
	}

	// Render per image tile
	PR::ToneMapper toneMapper;
	for (PR::uint32 i = 0; i < options.ImageTileXCount * options.ImageTileYCount; ++i) {
		auto renderer = renderFactory->create(i, options.ImageTileXCount, options.ImageTileYCount);
		if (options.ImageTileXCount * options.ImageTileYCount == 1) {
			PR_LOG(PR::L_INFO) << "Starting rendering of image ["
							   << renderer->offsetX() << ", " << (renderer->offsetX() + renderer->width()) << "] x ["
							   << renderer->offsetY() << ", " << (renderer->offsetY() + renderer->height()) << "]" << std::endl;
		} else {
			PR_LOG(PR::L_INFO) << "Starting rendering of image tile " << (renderer->index() + 1) << "/" << (options.ImageTileXCount * options.ImageTileYCount) << "["
							   << renderer->offsetX() << ", " << (renderer->offsetX() + renderer->width()) << "] x ["
							   << renderer->offsetY() << ", " << (renderer->offsetY() + renderer->height()) << "]" << std::endl;
		}

		env->outputSpecification().setup(renderer);

		if (options.ShowInformation)
			env->dumpInformation();

		if (options.ShowProgress)
			std::cout << "preprocess" << std::endl;

		renderer->start(options.RenderTileXCount, options.RenderTileYCount, options.ThreadCount);

		auto start		= sc::high_resolution_clock::now();
		auto start_prog = start;
		auto start_img  = start;
		while (!renderer->isFinished()) {
			auto end	   = sc::high_resolution_clock::now();
			auto span_prog = sc::duration_cast<sc::seconds>(end - start_prog);
			if (options.ShowProgress > 0 && span_prog.count() > options.ShowProgress) {
				PR::RenderStatus status = renderer->status();

				std::cout << std::setw(OUTPUT_FIELD_SIZE) << /*std::setfill('0') <<*/ std::setprecision(4) << std::fixed << status.percentage() * 100 << "%"
						  << " Pass " << renderer->currentPass() + 1;

				if (status.hasField("int.feedback"))
					std::cout << "( " << status.getField("int.feedback").getString() << ")";

				std::cout << " | S: " << std::setw(OUTPUT_FIELD_SIZE) << status.getField("global.pixel_sample_count").getUInt()
						  << " R: " << std::setw(OUTPUT_FIELD_SIZE) << status.getField("global.ray_count").getUInt()
						  << " EH: " << std::setw(OUTPUT_FIELD_SIZE) << status.getField("global.entity_hit_count").getUInt()
						  << " BH: " << std::setw(OUTPUT_FIELD_SIZE) << status.getField("global.background_hit_count").getUInt()
						  << std::endl;

				start_prog = end;
			}

			auto span_img = sc::duration_cast<sc::milliseconds>(end - start_img);
			if (options.DDO == DDO_Image && options.ImgUpdate > 0 && span_img.count() > options.ImgUpdate * 1000) {
				env->save(renderer, toneMapper, false);
				start_img = end;
			}
		}

		{
			auto end  = sc::high_resolution_clock::now();
			auto span = sc::duration_cast<sc::seconds>(end - start);

			if (!options.IsQuiet && options.ShowProgress && span.count() >= 1)
				std::cout << std::endl;

			PR_LOG(PR::L_INFO) << "Rendering took " << span.count() << " seconds" << std::endl;
		}

		// Save images
		env->save(renderer, toneMapper, true);
	}

	env->outputSpecification().deinit();

	return 0;
}
