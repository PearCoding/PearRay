#include "Environment.h"
#include "SceneLoader.h"

#include "renderer/RenderFactory.h"
#include "renderer/RenderContext.h"
#include "Test.h"

using namespace PR;

/* Little test to ensure same energy is produced by the several integrators
 * (which is unfortunatly not the case currently)
 */

const char* PROJECT = R"(
(scene
	:name 'project'
	:renderWidth 100
	:renderHeight 100
	:camera 'Camera'
	(output
		:name 'image'
		(channel
			:type 'rgb'
			:gamma 'False'
			:mapper 'none'
		)
	)
	(output
		:name 'depth'
		(channel
			:type 'depth'
		)
	)
	(entity
		:name 'Camera'
		:type 'camera'
		:width 1.000000
		:height 1.000000
		:zoom 1.000000
		:fstop 0.000000
		:apertureRadius 0.500000
		:localDirection [0,0,-1]
		:localUp [0,-1,0]
		:localRight [1,0,0]
		:position [0.000000,-2.000000,0.000000]
		:rotation (euler 90.0000,0.0000,0.0000)
		:scale [1.000000,1.000000,1.000000]
	)
	(spectrum
		:name 'Lamp_color'
		:data (rgb 1.000000 1.000000 1.000000)
	)
	(material
		:name 'Lamp_mat'
		:type 'light'
		:camera_visible false
		:emission 'Lamp_color'
	)
	(entity
		:name 'Lamp'
		:type 'sphere'
		:radius 0.100000
		:material 'Lamp_mat'
		:position [1.000000,-1.000000,1.000000]
	)
	(mesh
		:name 'Plane'
		:type 'triangles'
		(attribute
			:type 'p', [-1.000000, -1.000000, 0.000000], [1.000000, -1.000000, 0.000000], [1.000000, 1.000000, 0.000000], [-1.000000, 1.000000, 0.000000]
		)
		(attribute
			:type 'n', [0.000000, 0.000000, 1.000000], [0.000000, 0.000000, 1.000000], [0.000000, 0.000000, 1.000000], [0.000000, 0.000000, 1.000000]
		)
		(materials, 0, 0
		)
		(faces, 0, 1, 2, 0, 2, 3
		)
	)
	(entity
		:name 'Plane'
		:type 'mesh'
		:materials 'Material'
		:mesh 'Plane'
		:position [0.000000,0.000000,0.000000]
		:rotation (euler 90.0000,-0.0000,-0.0000)
		:scale [1.000000,1.000000,1.000000]
	)
	(spectrum
		:name 'Material_diffuse_color'
		:data (rgb 1.000000 1.000000 1.000000)
	)
	(material
		:name 'Material'
		:type 'diffuse'
		:albedo 'Material_diffuse_color'
	)
))";

constexpr int SPX = 50;
constexpr int SPY = 50;
constexpr float EPS = 10.0f;
constexpr int THREADS = 0;

PR_BEGIN_TESTCASE(IntEqEn)
PR_TEST("Equal Energy")
{
	auto env = SceneLoader::loadFromString(PROJECT);
	PR_ASSERT(env, "Test project string should be valid");

	auto renderFactory = std::make_shared<RenderFactory>(
		env->renderWidth(),
		env->renderHeight(),
		env->scene(), "", true);

	env->scene().freeze();
	env->scene().buildTree();

	Spectrum diOutput;
	Spectrum bidiOutput;
	Spectrum ppmOutput;

	renderFactory->settings().setSeed(42);
	renderFactory->settings().setMaxDiffuseBounces(0);

	{// Direct
		renderFactory->settings().setIntegratorMode(IM_Direct);

		auto renderer = renderFactory->create(0, 1, 1);
		env->outputSpecification().setup(renderer);
		env->scene().setup(renderer);

		renderer->start(8, 8, THREADS);

		while(!renderer->isFinished())
		{}

		diOutput = renderer->output()->getFragment(Eigen::Vector2i(SPX,SPY));
	}

	{// Bi-Direct
		renderFactory->settings().setIntegratorMode(IM_BiDirect);

		auto renderer = renderFactory->create(0, 1, 1);
		env->outputSpecification().setup(renderer);
		env->scene().setup(renderer);

		renderer->start(8, 8, THREADS);

		while(!renderer->isFinished())
		{}

		bidiOutput = renderer->output()->getFragment(Eigen::Vector2i(SPX,SPY));
	}

	{// PPM
		renderFactory->settings().setIntegratorMode(IM_PPM);
		renderFactory->settings().ppm().setMaxPhotonsPerPass(200000);
		renderFactory->settings().ppm().setMaxPassCount(10);

		auto renderer = renderFactory->create(0, 1, 1);
		env->outputSpecification().setup(renderer);
		env->scene().setup(renderer);

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