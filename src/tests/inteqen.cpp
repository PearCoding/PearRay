#include "Environment.h"
#include "SceneLoader.h"

#include "Logger.h"

#include "renderer/RenderFactory.h"
#include "renderer/RenderContext.h"
#include "Test.h"

using namespace PR;

/* Little test to ensure same energy is produced by the several integrators
 * (which is unfortunatly not the case currently)
 */

const char* PROJECT =
#include "testscene.inl"
;

constexpr int SPX = 50;
constexpr int SPY = 50;
constexpr float EPS = 10.0f;
constexpr int THREADS = 0;

PR_BEGIN_TESTCASE(IntEqEn)
PR_TEST("Equal Energy")
{
	auto env = SceneLoader::loadFromString(PROJECT);
	PR_ASSERT(env, "Test project string should be valid");

	std::shared_ptr<SpectrumDescriptor> specDesc = SpectrumDescriptor::createStandardSpectral();
	auto scene = env->sceneFactory().create();
	auto renderFactory = std::make_shared<RenderFactory>(
		specDesc,
		env->renderWidth(),
		env->renderHeight(),
		scene, "");

	Spectrum diOutput(specDesc);
	Spectrum bidiOutput(specDesc);
	Spectrum ppmOutput(specDesc);

	renderFactory->settings().setSeed(42);
	renderFactory->settings().setMaxDiffuseBounces(0);

	std::cout << "Direct>" << std::endl;
	{
		renderFactory->settings().setIntegratorMode(IM_Direct);

		auto renderer = renderFactory->create(0, 1, 1);
		env->outputSpecification().setup(renderer);

		renderer->start(8, 8, THREADS);

		while(!renderer->isFinished())
		{}

		diOutput = renderer->output()->getFragment(Eigen::Vector2i(SPX,SPY));
	}

	std::cout << "Bi-Direct>" << std::endl;
	{
		renderFactory->settings().setIntegratorMode(IM_BiDirect);

		auto renderer = renderFactory->create(0, 1, 1);
		env->outputSpecification().setup(renderer);

		renderer->start(8, 8, THREADS);

		while(!renderer->isFinished())
		{}

		bidiOutput = renderer->output()->getFragment(Eigen::Vector2i(SPX,SPY));
	}

	std::cout << "PPM>" << std::endl;
	{
		renderFactory->settings().setIntegratorMode(IM_PPM);
		renderFactory->settings().ppm().setMaxPhotonsPerPass(200000);
		renderFactory->settings().ppm().setMaxPassCount(10);

		auto renderer = renderFactory->create(0, 1, 1);
		env->outputSpecification().setup(renderer);

		renderer->start(8, 8, THREADS);

		while(!renderer->isFinished())
		{}

		ppmOutput = renderer->output()->getFragment(Eigen::Vector2i(SPX,SPY));
	}

	const float dif = diOutput.luminousFlux();
	const float bidif = bidiOutput.luminousFlux();
	const float ppmf = ppmOutput.luminousFlux();

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