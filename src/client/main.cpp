#include "Environment.h"
#include "Logger.h"
#include "Profiler.h"
#include "ProgramSettings.h"
#include "SceneLoader.h"
#include "Utils.h"
#include "config/Build.h"
#include "log/FileLogListener.h"
#include "renderer/RenderContext.h"
#include "renderer/RenderFactory.h"
#include "renderer/RenderStatistics.h"
#include "spectral/ToneMapper.h"

#include "ImageUpdateObserver.h"
#include "NetworkObserver.h"
#include "StatusObserver.h"
#include "TevObserver.h"

#include <filesystem>

#include <chrono>
#include <csignal>
#include <iomanip>
#include <iostream>
#include <thread>

namespace sf = std::filesystem;
namespace sc = std::chrono;

using namespace PR;

constexpr int OUTPUT_FIELD_SIZE = 8;

void printStatistics(const RenderStatus& status)
{
	auto& out = PR_LOG(L_INFO);
	out << "Statistics:" << std::endl;

	const auto raycount	  = std::get<uint64>(status.getField("global.ray_count"));
	const auto pixelcount = std::get<uint64>(status.getField("global.pixel_sample_count"));

	out << "  Ray Count:         " << std::setw(OUTPUT_FIELD_SIZE) << raycount << std::endl
		<< "  │ Sources:" << std::endl
		<< "  │ ├ Camera Share:  " << std::setw(OUTPUT_FIELD_SIZE) << 100 * std::get<uint64>(status.getField("global.camera_ray_count")) / (double)raycount << " %" << std::endl
		<< "  │ └ Light Share:   " << std::setw(OUTPUT_FIELD_SIZE) << 100 * std::get<uint64>(status.getField("global.light_ray_count")) / (double)raycount << " %" << std::endl
		<< "  ├ Types:" << std::endl
		<< "  │ ├ Primary Share: " << std::setw(OUTPUT_FIELD_SIZE) << 100 * std::get<uint64>(status.getField("global.primary_ray_count")) / (double)raycount << " %" << std::endl
		<< "  │ ├ Bounce Share:  " << std::setw(OUTPUT_FIELD_SIZE) << 100 * std::get<uint64>(status.getField("global.bounce_ray_count")) / (double)raycount << " %" << std::endl
		<< "  │ └ Shadow Share:  " << std::setw(OUTPUT_FIELD_SIZE) << 100 * std::get<uint64>(status.getField("global.shadow_ray_count")) / (double)raycount << " %" << std::endl
		<< "  └ Monochrome:      " << std::setw(OUTPUT_FIELD_SIZE) << 100 * std::get<uint64>(status.getField("global.monochrome_ray_count")) / (double)raycount << " %" << std::endl
		<< "  Pixel Count:       " << std::setw(OUTPUT_FIELD_SIZE) << pixelcount << std::endl
		<< "  Entity Hits:       " << std::setw(OUTPUT_FIELD_SIZE) << std::get<uint64>(status.getField("global.entity_hit_count")) << std::endl
		<< "  Background Hits:   " << std::setw(OUTPUT_FIELD_SIZE) << std::get<uint64>(status.getField("global.background_hit_count")) << std::endl
		<< "  Mean Ray Depth:    " << std::setw(OUTPUT_FIELD_SIZE) << std::get<uint64>(status.getField("global.depth_count")) / (double)pixelcount << std::endl
		<< "  ├ Camera:          " << std::setw(OUTPUT_FIELD_SIZE) << std::get<uint64>(status.getField("global.camera_depth_count")) / (double)pixelcount << std::endl
		<< "  └ Light:           " << std::setw(OUTPUT_FIELD_SIZE) << std::get<uint64>(status.getField("global.light_depth_count")) / (double)pixelcount << std::endl
		<< "  Iterations:        " << std::setw(OUTPUT_FIELD_SIZE) << std::get<uint64>(status.getField("global.iteration_count")) << std::endl;
}

static uint32 sForceStop			 = 0;
static uint32 sInterruptCounter		 = 0;
constexpr uint32 HARD_STOP_THRESHOLD = 5;  // After this count, the renderer will try to hard stop (inbetween iterations) execution
constexpr uint32 MAX_INTERRUPTS		 = 15; // After this count, the renderer will abort execution
void pr_signalHandler(int signal)
{
	++sInterruptCounter;
	if (sInterruptCounter > MAX_INTERRUPTS) {
		std::cerr << std::endl
				  << "Aborting renderer!" << std::endl
				  << std::endl;
		exit(EXIT_FAILURE);
	} else if (signal == SIGTERM || sInterruptCounter > HARD_STOP_THRESHOLD) {
		if (sForceStop != 2)
			std::cout << std::endl
					  << "Trying to stop the renderer" << std::endl
					  << std::endl;
		sForceStop = 2;
	} else {
		if (sForceStop != 1)
			std::cout << std::endl
					  << "Trying to soft stop the renderer" << std::endl
					  << std::endl;
		sForceStop = 1;
	}
}

constexpr uint32 PROFILE_SAMPLE_RATE = 10;
int main(int argc, char** argv)
{
	ProgramSettings options;
	if (!options.parse(argc, argv))
		return EXIT_FAILURE;

	// Setup signal handler
	if (signal(SIGINT, pr_signalHandler) == SIG_ERR || signal(SIGTERM, pr_signalHandler) == SIG_ERR)
		std::cout << "Error while setting signal handler. Stopping progressive rendering through console might not be possible" << std::endl;

	if (options.Profile)
		Profiler::start(PROFILE_SAMPLE_RATE);

	time_t t = time(NULL);
	std::stringstream sstream;
#ifdef PR_DEBUG
	sstream << "pr_" << t << "_d.log";
#else
	sstream << "pr_" << t << ".log";
#endif
	const sf::path logFile = options.OutputDir / sstream.str();

	// If the plugin path is empty, use the current working directory
	if (options.PluginPath.empty())
		options.PluginPath = sf::current_path().string();

	// Setup log
	std::shared_ptr<FileLogListener> fileLogListener = std::make_shared<FileLogListener>();
	fileLogListener->open(logFile.string());
	PR_LOGGER.addListener(fileLogListener);

	PR_LOGGER.setQuiet(options.IsQuiet);
	PR_LOGGER.enableAnsiTerminal(!options.NoPrettyConsole);
	PR_LOGGER.setVerbosity(options.IsVerbose ? L_DEBUG : L_INFO);

	if (!options.IsQuiet)
		std::cout << Build::getCopyrightString() << std::endl;

	if (options.Profile) {
		PR_LOG(L_INFO) << "Profiling enabled (TpS: "
					   << std::chrono::high_resolution_clock::period::den
					   << ")" << std::endl;
	}

	// Load scene
	SceneLoader::LoadOptions opts;
	opts.WorkingDir	 = options.OutputDir.generic_wstring();
	opts.PluginPath	 = options.PluginPath.generic_wstring();
	opts.CacheMode	 = options.CacheMode;
	opts.Progressive = options.Progressive;

	const std::shared_ptr<Environment> env = SceneLoader::loadFromFile(
		options.InputFile.generic_wstring(),
		opts);

	if (!env) {
		PR_LOG(L_ERROR) << "Error while parsing input." << std::endl;
		return EXIT_FAILURE;
	}

	env->renderSettings().useAdaptiveTiling = options.AdaptiveTiling;
	env->renderSettings().sortHits			= options.SortHits;

	// Initialize observers
	std::vector<std::unique_ptr<IProgressObserver>> observers;
	if (options.ImgUpdate > 0 || options.ImgUpdateIteration > 0)
		observers.push_back(std::make_unique<ImageUpdateObserver>(env.get()));
	if (options.ShowProgress > 0)
		observers.push_back(std::make_unique<StatusObserver>());
	if (options.ListenNetwork > 0)
		observers.push_back(std::make_unique<NetworkObserver>());
	if (options.TevUpdate > 0)
		observers.push_back(std::make_unique<TevObserver>());

	// Setup renderFactory
	const auto renderFactory = env->createRenderFactory();
	if (!renderFactory) {
		PR_LOG(L_ERROR) << "Could not setup render factory." << std::endl;
		return EXIT_FAILURE;
	}

	const auto integrator = env->createSelectedIntegrator();

	// Render per image tile
	ToneMapper toneMapper;
	for (uint32 i = 0; i < options.ImageTileXCount * options.ImageTileYCount; ++i) {
		const auto renderer = renderFactory->create(integrator, i, Size2i(options.ImageTileXCount, options.ImageTileYCount));

		if (!renderer) {
			PR_LOG(L_ERROR) << "Unable to create renderer!" << std::endl;
			return EXIT_FAILURE;
		}

		const auto outputDevice = env->createAndAssignFrameOutputDevice(renderer);

		// Make sure output is configured for output
		// TODO: This is bad design -> Encapsulate rendercontext dependent parts
		env->setup(renderer);

		if (options.ImageTileXCount * options.ImageTileYCount == 1) {
			PR_LOG(L_INFO) << "Starting rendering of image ("
						   << PR_FMT_MAT(renderer->viewOffset()) << ", " << PR_FMT_MAT(renderer->viewOffset() + renderer->viewSize()) << ")" << std::endl;
		} else {
			PR_LOG(L_INFO) << "Starting rendering of image tile " << (renderer->index() + 1) << "/" << (options.ImageTileXCount * options.ImageTileYCount)
						   << "(" << PR_FMT_MAT(renderer->viewOffset()) << ", " << PR_FMT_MAT(renderer->viewOffset() + renderer->viewSize()) << ")" << std::endl;
		}

		if (options.ShowInformation)
			env->dumpInformation();

		// Status variables
		uint32 maxIterations = 0;
		const auto start	 = sc::high_resolution_clock::now();

		// Setup observers
		for (const auto& obs : observers)
			obs->begin(renderer.get(), outputDevice.get(), options);

		// Setup iteration callback
		renderer->addIterationCallback([&](const RenderIteration& iter) {
			maxIterations = std::max(maxIterations, iter.Iteration);

			for (const auto& obs : observers)
				obs->onIteration(UpdateInfo{ start, iter.Iteration, iter.Pass });
		});

		renderer->start(options.RenderTileXCount, options.RenderTileYCount, options.ThreadCount);

		while (!renderer->isFinished()) {
			std::this_thread::sleep_for(sc::milliseconds(500));

			const auto end		 = sc::high_resolution_clock::now();
			const auto span_full = sc::duration_cast<sc::seconds>(end - start);

			for (const auto& obs : observers)
				obs->update(UpdateInfo{ start, maxIterations, 0 });

			if (sForceStop == 1)
				renderer->requestSoftStop();
			else if (sForceStop == 2)
				renderer->requestStop();

			if (options.MaxTime > 0 && span_full.count() >= options.MaxTime) {
				if (options.MaxTimeForce)
					renderer->requestStop();
				else
					renderer->requestSoftStop();
			}

			if (renderer->isStopping())
				renderer->waitForFinish();
		}

		for (const auto& obs : observers)
			obs->end();

		renderer->notifyEnd();

		{
			const auto end	= sc::high_resolution_clock::now();
			const auto span = sc::duration_cast<sc::seconds>(end - start);
			PR_LOG(L_INFO) << "Rendering took " << timestr(span.count()) << std::endl;

			// Save images
			OutputSaveOptions output_options;
			output_options.Image.IterationMeta = maxIterations;
			output_options.Image.TimeMeta	   = span.count();
			output_options.Image.WriteMeta	   = true;
			// output_options.NameSuffix = ""; // (Do not make use of tags)
			env->save(renderer.get(), outputDevice.get(), toneMapper, output_options);
		}

		// Print Statistics
		if (!options.IsQuiet)
			printStatistics(renderer->status());

		if (sForceStop != 0)
			break;
	}

	observers.clear();
	env->outputSpecification().deinit();

	if (options.Profile) {
		Profiler::stop();
		const sf::path profFile = options.OutputDir / "pr_profile.prof";
		if (!Profiler::dumpToFile(profFile.generic_wstring()))
			PR_LOG(L_ERROR) << "Could not write profile data to " << profFile << std::endl;
	}

	return EXIT_SUCCESS;
}
