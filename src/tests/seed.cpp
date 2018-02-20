#include "Environment.h"
#include "SceneLoader.h"

#include "Test.h"
#include "renderer/RenderContext.h"
#include "renderer/RenderFactory.h"

using namespace PR;

/* Little test to ensure equal seed produces equal output
 * (which is unfortunatly not the case currently)
 */

const char* PROJECT =
#include "testscene.inl"
	;

constexpr uint32 POS_COUNT   = 10;
const uint32 xpos[POS_COUNT] = { 5, 7, 9, 10, 24, 34, 42, 50, 68, 76 };
const uint32 ypos[POS_COUNT] = { 2, 6, 10, 11, 35, 42, 69, 78, 90, 99 };
constexpr float EPS			 = 0.0001f;
constexpr int THREADS		 = 0;
constexpr uint64 SEED		 = 42;

PR_BEGIN_TESTCASE(Seed)
PR_TEST("Direct Integrator")
{
	auto env = SceneLoader::loadFromString(PROJECT);
	PR_ASSERT(env, "Test project string should be valid");

	auto scene = env->sceneFactory().create();
	auto renderFactory = std::make_shared<RenderFactory>(
		SpectrumDescriptor::createStandardSpectral(),
		env->renderWidth(),
		env->renderHeight(),
		scene, "");

	std::list<Spectrum> output1;
	std::list<Spectrum> output2;

	renderFactory->settings().setSeed(SEED);
	renderFactory->settings().setMaxDiffuseBounces(0);

	{ // 1
		renderFactory->settings().setIntegratorMode(IM_Direct);

		auto renderer = renderFactory->create(0, 1, 1);
		env->outputSpecification().setup(renderer);

		renderer->start(8, 8, THREADS);

		while (!renderer->isFinished()) {
		}

		for (uint32 i  = 0; i < POS_COUNT; ++i) {
			Spectrum spec = renderer->output()->getFragment(Eigen::Vector2i(xpos[i], ypos[i])).clone();
			output1.push_back(spec);
		}
	}

	{ // 2
		renderFactory->settings().setIntegratorMode(IM_Direct);

		auto renderer = renderFactory->create(0, 1, 1);
		env->outputSpecification().setup(renderer);

		renderer->start(8, 8, THREADS);

		while (!renderer->isFinished()) {
		}

		for (uint32 i  = 0; i < POS_COUNT; ++i) {
			Spectrum spec = renderer->output()->getFragment(Eigen::Vector2i(xpos[i], ypos[i])).clone();
			output2.push_back(spec);
		}
	}

	auto it1 = output1.begin();
	auto it2 = output2.begin();
	uint32 i = 0;
	while(it1 != output1.end() && it2 != output2.end()) {
		const float dif = Spectrum(*it1 - *it2).sqrSum();
		std::cout << "DI [" << xpos[i] << "|" << ypos[i] << "] " << dif 
				  << " (" << it1->max() << "|" << it2->max() << ")" << std::endl;
		PR_CHECK_NEARLY_EQ_EPS(dif, 0, EPS);

		++it1;
		++it2;
		++i;
	}
}

PR_TEST("Bi-Direct Integrator")
{
	auto env = SceneLoader::loadFromString(PROJECT);
	PR_ASSERT(env, "Test project string should be valid");

	auto scene = env->sceneFactory().create();
	auto renderFactory = std::make_shared<RenderFactory>(
		SpectrumDescriptor::createStandardSpectral(),
		env->renderWidth(),
		env->renderHeight(),
		scene, "");

	std::list<Spectrum> output1;
	std::list<Spectrum> output2;

	renderFactory->settings().setSeed(SEED);
	renderFactory->settings().setMaxDiffuseBounces(0);

	{ // 1
		renderFactory->settings().setIntegratorMode(IM_BiDirect);

		auto renderer = renderFactory->create(0, 1, 1);
		env->outputSpecification().setup(renderer);

		renderer->start(8, 8, THREADS);

		while (!renderer->isFinished()) {
		}

		for (uint32 i  = 0; i < POS_COUNT; ++i) {
			Spectrum spec = renderer->output()->getFragment(Eigen::Vector2i(xpos[i], ypos[i])).clone();
			output1.push_back(spec);
		}
	}

	{ // 2
		renderFactory->settings().setIntegratorMode(IM_BiDirect);

		auto renderer = renderFactory->create(0, 1, 1);
		env->outputSpecification().setup(renderer);

		renderer->start(8, 8, THREADS);

		while (!renderer->isFinished()) {
		}

		for (uint32 i  = 0; i < POS_COUNT; ++i) {
			Spectrum spec = renderer->output()->getFragment(Eigen::Vector2i(xpos[i], ypos[i])).clone();
			output2.push_back(spec);
		}
	}

	auto it1 = output1.begin();
	auto it2 = output2.begin();
	uint32 i = 0;
	while(it1 != output1.end() && it2 != output2.end()) {
		const float dif = Spectrum(*it1 - *it2).sqrSum();
		std::cout << "BIDI [" << xpos[i] << "|" << ypos[i] << "] " << dif 
				  << " (" << it1->max() << "|" << it2->max() << ")" << std::endl;
		PR_CHECK_NEARLY_EQ_EPS(dif, 0, EPS);

		++it1;
		++it2;
		++i;
	}
}

/*PR_TEST("PPM Integrator")
{
	auto env = SceneLoader::loadFromString(PROJECT);
	PR_ASSERT(env, "Test project string should be valid");

	auto scene = env->sceneFactory().create();
	auto renderFactory = std::make_shared<RenderFactory>(
		env->renderWidth(),
		env->renderHeight(),
		scene, "");

	Spectrum output1[POS_COUNT];
	Spectrum output2[POS_COUNT];

	renderFactory->settings().setSeed(SEED);
	renderFactory->settings().setMaxDiffuseBounces(0);

	{ // 1
		renderFactory->settings().setIntegratorMode(IM_PPM);
		renderFactory->settings().ppm().setMaxPhotonsPerPass(200000);
		renderFactory->settings().ppm().setMaxPassCount(10);

		auto renderer = renderFactory->create(0, 1, 1);
		env->outputSpecification().setup(renderer);

		renderer->start(8, 8, THREADS);

		while (!renderer->isFinished()) {
		}

		for (uint32 i  = 0; i < POS_COUNT; ++i)
			output1[i] = renderer->output()->getFragment(Eigen::Vector2i(xpos[i], ypos[i]));
	}

	{ // 2
		renderFactory->settings().setIntegratorMode(IM_PPM);
		renderFactory->settings().ppm().setMaxPhotonsPerPass(200000);
		renderFactory->settings().ppm().setMaxPassCount(10);

		auto renderer = renderFactory->create(0, 1, 1);
		env->outputSpecification().setup(renderer);
		env->scene().setup(renderer);

		renderer->start(8, 8, THREADS);

		while (!renderer->isFinished()) {
		}

		for (uint32 i  = 0; i < POS_COUNT; ++i)
			output2[i] = renderer->output()->getFragment(Eigen::Vector2i(xpos[i], ypos[i]));
	}

	for (uint32 i = 0; i < POS_COUNT; ++i) {
		const float dif = (output1[i] - output2[i]).sqrSum();
		std::cout << "PPM [" << xpos[i] << "|" << ypos[i] << "] " << dif << std::endl;
		PR_CHECK_NEARLY_EQ_EPS(dif, 0, EPS);
	}
}*/
PR_END_TESTCASE()

// MAIN
PRT_BEGIN_MAIN
PRT_TESTCASE(Seed);
PRT_END_MAIN