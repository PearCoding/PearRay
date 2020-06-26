#include "Environment.h"
#include "Logger.h"
#include "Profiler.h"
#include "ProgramSettings.h"
#include "SceneLoader.h"
#include "config/Build.h"
#include "log/FileLogListener.h"
#include "renderer/RenderContext.h"
#include "renderer/RenderFactory.h"
#include "renderer/RenderTileStatistics.h"
#include "spectral/ToneMapper.h"

#include <filesystem>

#include <chrono>
#include <iomanip>
#include <iostream>
#include <thread>

namespace sf = std::filesystem;
namespace sc = std::chrono;

using namespace PR;

constexpr int OUTPUT_FIELD_SIZE = 8;

void printStatus(const RenderStatus& status)
{
	if (status.hasField("int.feedback"))
		std::cout << "( " << status.getField("int.feedback").getString() << ")";

	std::cout << " | S: " << std::setw(OUTPUT_FIELD_SIZE) << status.getField("global.pixel_sample_count").getUInt()
			  << " R: " << std::setw(OUTPUT_FIELD_SIZE) << status.getField("global.ray_count").getUInt()
			  << " EH: " << std::setw(OUTPUT_FIELD_SIZE) << status.getField("global.entity_hit_count").getUInt()
			  << " BH: " << std::setw(OUTPUT_FIELD_SIZE) << status.getField("global.background_hit_count").getUInt();
}

void printStatistics(const RenderStatus& status)
{
	auto& out = PR_LOG(L_INFO);
	out << "Statistics:" << std::endl;

	const auto raycount	  = status.getField("global.ray_count").getUInt();
	const auto pixelcount = status.getField("global.pixel_sample_count").getUInt();

	out << "  Ray Count:       " << std::setw(OUTPUT_FIELD_SIZE) << raycount << std::endl;
	out << "    Camera Share:  " << std::setw(OUTPUT_FIELD_SIZE) << 100 * status.getField("global.camera_ray_count").getUInt() / (double)raycount << " %" << std::endl;
	out << "    Light Share:   " << std::setw(OUTPUT_FIELD_SIZE) << 100 * status.getField("global.light_ray_count").getUInt() / (double)raycount << " %" << std::endl;
	out << "    Bounce Share:  " << std::setw(OUTPUT_FIELD_SIZE) << 100 * status.getField("global.bounce_ray_count").getUInt() / (double)raycount << " %" << std::endl;
	out << "    Shadow share:  " << std::setw(OUTPUT_FIELD_SIZE) << 100 * status.getField("global.shadow_ray_count").getUInt() / (double)raycount << " %" << std::endl;
	out << "  Pixel Count:     " << std::setw(OUTPUT_FIELD_SIZE) << pixelcount << std::endl;
	out << "  Entity Hits:     " << std::setw(OUTPUT_FIELD_SIZE) << status.getField("global.entity_hit_count").getUInt() << std::endl;
	out << "  Background Hits: " << std::setw(OUTPUT_FIELD_SIZE) << status.getField("global.background_hit_count").getUInt() << std::endl;
	out << "  Mean Ray Depth:  " << std::setw(OUTPUT_FIELD_SIZE) << status.getField("global.depth_count").getUInt() / (double)pixelcount << std::endl;
}

std::string timestr(uint64 sec)
{
	if (sec == 0)
		return " <1s";

	std::stringstream sstream;

	uint64 s = sec % 60;
	sec /= 60;
	uint64 m = sec % 60;
	sec /= 60;
	uint64 h = sec;

	if (h > 0)
		sstream << h << "h ";

	if (m > 0)
		sstream << m << "m ";

	if (s > 0)
		sstream << s << "s ";

	return sstream.str();
}

constexpr uint32 PROFILE_SAMPLE_RATE = 10;
int main(int argc, char** argv)
{
	ProgramSettings options;
	if (!options.parse(argc, argv))
		return -1;

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
	if (options.PluginPath.empty()) {
		options.PluginPath = sf::current_path().string();
	}

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

	SceneLoader::LoadOptions opts;
	opts.WorkingDir					 = options.OutputDir.generic_wstring();
	opts.PluginPath					 = options.PluginPath.generic_wstring();
	opts.CacheMode					 = options.CacheMode;
	std::shared_ptr<Environment> env = SceneLoader::loadFromFile(
		options.InputFile.generic_wstring(),
		opts);

	if (!env) {
		if (!options.IsQuiet)
			PR_LOG(L_ERROR) << "Error while parsing input." << std::endl;

		return -2;
	}

	env->renderSettings().useAdaptiveTiling = options.AdaptiveTiling;
	env->renderSettings().sortHits			= options.SortHits;

	// Setup renderFactory
	auto renderFactory = env->createRenderFactory();
	if (!renderFactory) {
		if (!options.IsQuiet)
			PR_LOG(L_ERROR) << "Error: Couldn't setup render factory." << std::endl;

		return -4;
	}

	auto integrator = env->createSelectedIntegrator();

	// Render per image tile
	ToneMapper toneMapper;
	for (uint32 i = 0; i < options.ImageTileXCount * options.ImageTileYCount; ++i) {
		auto renderer = renderFactory->create(
			integrator,
			i,
			Size2i(options.ImageTileXCount, options.ImageTileYCount));

		if (!renderer) {
			PR_LOG(L_ERROR) << "Unable to create renderer!" << std::endl;
			return -5;
		}
		const auto maxIterationCount = renderer->maxIterationCount();

		std::atomic<bool> softStop(false); // Stop after iteration end

		uint32 imgIterCounter  = 0;
		uint32 fullIterCounter = 0;
		auto start			   = sc::high_resolution_clock::now();

		const auto saveImg = [&]() {
			OutputSaveOptions output_options;

			output_options.Image.IterationMeta	= fullIterCounter;
			output_options.Image.TimeMeta		= sc::duration_cast<sc::seconds>(sc::high_resolution_clock::now() - start).count();
			output_options.Image.WriteMeta		= true;
			output_options.Image.SpectralFactor = std::max(1.0f, maxIterationCount - fullIterCounter - 1.0f);

			if (options.ImgUseTags) {
				std::stringstream stream;
				stream << "_i" << output_options.Image.IterationMeta << "_t" << output_options.Image.TimeMeta;
				output_options.NameSuffix = stream.str();
			}

			env->save(renderer, toneMapper, output_options);
		};

		renderer->setIterationCallback([&](uint32 iter) {
			if (softStop.exchange(false)) {
				renderer->stop();
			}

			++fullIterCounter;
			++imgIterCounter;
			if (iter < maxIterationCount && options.ImgUpdateIteration > 0 && imgIterCounter >= options.ImgUpdateIteration) {
				saveImg();
				imgIterCounter = 0;
			}
		});

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

		if (options.ShowProgress)
			std::cout << "preprocess" << std::endl;

		renderer->start(options.RenderTileXCount, options.RenderTileYCount, options.ThreadCount);

		auto start_prog = start;
		auto start_img	= start;
		while (!renderer->isFinished()) {
			std::this_thread::sleep_for(sc::seconds(1));

			const auto end		 = sc::high_resolution_clock::now();
			const auto span_prog = sc::duration_cast<sc::seconds>(end - start_prog);
			const auto span_full = sc::duration_cast<sc::seconds>(end - start);
			const auto span_img	 = sc::duration_cast<sc::seconds>(end - start_img);

			if (options.ShowProgress > 0 && span_prog.count() >= options.ShowProgress) {
				RenderStatus status = renderer->status();

				if (!options.NoPrettyConsole)
					std::cout << "\r";

				std::cout << std::setw(OUTPUT_FIELD_SIZE) << std::setprecision(4) << std::fixed
						  << status.percentage() * 100 << "%";
				printStatus(status);
				std::cout << " | RT: " << std::setw(OUTPUT_FIELD_SIZE) << timestr(span_full.count())
						  << " ETA: " << std::setw(OUTPUT_FIELD_SIZE) << timestr(span_full.count() * ((1 - status.percentage()) / status.percentage()));

				if (options.NoPrettyConsole)
					std::cout << std::endl;
				else
					std::cout << std::flush;

				start_prog = end;
			}

			if (options.ImgUpdate > 0 && span_img.count() >= options.ImgUpdate) {
				saveImg();
				start_img = end;
			}

			if (options.MaxTime > 0 && span_full.count() >= options.MaxTime) {
				if (options.MaxTimeForce)
					renderer->stop();
				else
					softStop = true;
			}
		}

		if (options.ShowProgress > 0) {
			if (!options.NoPrettyConsole)
				std::cout << "\r";
			std::cout << "Done" << std::setw(120) << " " << std::endl;
		}

		renderer->notifyEnd();

		{
			const auto end	= sc::high_resolution_clock::now();
			const auto span = sc::duration_cast<sc::seconds>(end - start);
			PR_LOG(L_INFO) << "Rendering took " << timestr(span.count()) << std::endl;

			// Save images
			OutputSaveOptions output_options;
			output_options.Image.IterationMeta = fullIterCounter;
			output_options.Image.TimeMeta	   = span.count();
			output_options.Image.WriteMeta	   = true;
			// output_options.NameSuffix = ""; // (Do not make use of tags)
			env->save(renderer, toneMapper, output_options);
		}

		// Print Statisticss
		if (!options.IsQuiet) {
			printStatistics(renderer->status());
		}
	}

	env->outputSpecification().deinit();

	if (options.Profile) {
		Profiler::stop();
		const sf::path profFile = options.OutputDir / "pr_profile.prof";
		if (!Profiler::dumpToFile(profFile.generic_wstring()))
			PR_LOG(L_ERROR) << "Could not write profile data to " << profFile << std::endl;
	}

	return 0;
}
