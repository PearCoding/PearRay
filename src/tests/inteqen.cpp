#include "Environment.h"
#include "SceneLoader.h"

#include "Logger.h"

#include "Test.h"
#include "renderer/RenderContext.h"
#include "renderer/RenderFactory.h"

using namespace PR;

/* Little test to ensure same energy is produced by the several integrators
 * (which is unfortunatly not the case currently)
 */

const char* PROJECT =
#include "testscene.inl"
	;

constexpr uint32 SEED = 42;
constexpr int SPX	 = 50;
constexpr int SPY	 = 50;
constexpr float EPS   = 10.0f;
constexpr int THREADS = 0;

PR_BEGIN_TESTCASE(IntEqEn)
PR_TEST("Equal Energy")
{
	auto env = SceneLoader::loadFromString("", PROJECT);
	PR_ASSERT(env, "Test project string should be valid");

	env->renderManager().registry()->setByGroup(RG_RENDERER, "common/seed", SEED);
	auto renderFactory = env->renderManager().createRenderFactory();

	Spectrum diOutput(renderFactory->spectrumDescriptor());
	Spectrum bidiOutput(renderFactory->spectrumDescriptor());
	Spectrum ppmOutput(renderFactory->spectrumDescriptor());

	std::cout << "Direct>" << std::endl;
	{
		env->renderManager().registry()->setByGroup(RG_RENDERER, "common/type", IM_DIRECT);
		env->renderManager().registry()->setByGroup(RG_RENDERER, "direct/diffuse/max_depth", 0);

		auto renderer = renderFactory->create(0, 1, 1);
		env->outputSpecification().setup(renderer);

		renderer->start(8, 8, THREADS);

		while (!renderer->isFinished()) {
		}

		renderer->output()->getFragment(Vector2i(SPX, SPY), diOutput);
	}

	std::cout << "Bi-Direct>" << std::endl;
	{
		env->renderManager().registry()->setByGroup(RG_RENDERER, "common/type", IM_BIDIRECT);
		env->renderManager().registry()->setByGroup(RG_RENDERER, "bidirect/diffuse/max_depth", 0);

		auto renderer = renderFactory->create(0, 1, 1);
		env->outputSpecification().setup(renderer);

		renderer->start(8, 8, THREADS);

		while (!renderer->isFinished()) {
		}

		renderer->output()->getFragment(Vector2i(SPX, SPY), bidiOutput);
	}

	std::cout << "PPM>" << std::endl;
	{
		env->renderManager().registry()->setByGroup(RG_RENDERER, "common/type", IM_PPM);
		env->renderManager().registry()->setByGroup(RG_RENDERER, "ppm/diffuse/max_depth", 0);
		env->renderManager().registry()->setByGroup(RG_RENDERER, "ppm/photons/pass_count", 10);

		auto renderer = renderFactory->create(0, 1, 1);
		env->outputSpecification().setup(renderer);

		renderer->start(8, 8, THREADS);

		while (!renderer->isFinished()) {
		}

		renderer->output()->getFragment(Vector2i(SPX, SPY), ppmOutput);
	}

	const float dif   = diOutput.luminousFlux();
	const float bidif = bidiOutput.luminousFlux();
	const float ppmf  = ppmOutput.luminousFlux();

	std::cout << "DI:   " << dif << std::endl;
	std::cout << "BIDI: " << bidif << std::endl;
	std::cout << "PPM:  " << ppmf << std::endl;

	PR_CHECK_NEARLY_EQ_EPS(dif, bidif, EPS);
	PR_CHECK_NEARLY_EQ_EPS(dif, ppmf, EPS);
	PR_CHECK_NEARLY_EQ_EPS(bidif, ppmf, EPS);
}
PR_END_TESTCASE()

// MAIN
PRT_BEGIN_MAIN
PRT_TESTCASE(IntEqEn);
PRT_END_MAIN