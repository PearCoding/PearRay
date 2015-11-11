#include "FileLogListener.h"

#include "scene/Camera.h"
#include "scene/Scene.h"

#include "entity/SphereEntity.h"

#include "renderer/Renderer.h"

#include "spectral/RGBConverter.h"

#include "material/DiffuseMaterial.h"

// PearPic
#include "PearPic.h"
#include "Image.h"

#ifdef PR_OS_WINDOWS
# define _CRTDBG_MAP_ALLOC
# include <stdlib.h>
# include <crtdbg.h>
#endif
#include <sstream>
#include <iostream>

int main(int argc, char** argv)
{
#ifdef PR_OS_WINDOWS
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF | _CRTDBG_CHECK_ALWAYS_DF);
	_CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_DEBUG);
#endif

	srand((unsigned int)time(NULL));

	// Init logging system
	PR::FileLogListener fileLog;

	time_t t = time(NULL);
	std::stringstream sstream;
#ifdef PR_DEBUG
	sstream << t << "_d.log";
#else
	sstream << t << ".log";
#endif
	fileLog.open(sstream.str());
	PR_LOGGER.addListener(&fileLog);

	// Init PearPic extensions
	PP::PearPic pearpic;
	pearpic.init();

	PR::Camera camera("Test_Cam");
	camera.setWithSize(1, 1, 0.2f);

	PR::Scene scene("Test");
	PR::Renderer renderer(500, 500, &camera, &scene);

	PR::Spectrum diffSpec;
	diffSpec.setValueAtWavelength(600, 1);

	PR::Spectrum emitSpec;
	emitSpec.setValueAtWavelength(600, 0.5f);
	emitSpec.setValueAtWavelength(540, 1);

	PR::DiffuseMaterial material(diffSpec);
	material.setRoughness(0.8f);

	PR::DiffuseMaterial material2(diffSpec);
	material2.setEmission(emitSpec);

	for (int x = 0; x <= 5; ++x)
	{
		for (int y = 0; y <= 5; ++y)
		{
			PR::SphereEntity* e = new PR::SphereEntity("Sphere", 2);
			e->setPosition(PM::pm_Set((float)x*4-10, (float)y*4-10, 5));
			e->setMaterial(&material);
			scene.addEntity(e);
		}
	}

	// Light
	PR::SphereEntity* e = new PR::SphereEntity("LightSphere", 0.5f);
	e->setPosition(PM::pm_Set(0, 0, 2));
	e->setMaterial(&material2);
	scene.addEntity(e);

	scene.buildTree();

	PR_LOGGER.log(PR::L_Info, PR::M_Scene, "Starting rendering...");
	PR::TimePoint renderStart = std::chrono::high_resolution_clock::now();
	renderer.start(5,5,2);

	PR::TimePoint pt = renderStart;
	while (!renderer.isFinished())
	{
		PR::TimePoint newT = std::chrono::high_resolution_clock::now();
		std::chrono::milliseconds dur =
			std::chrono::duration_cast<std::chrono::milliseconds>(newT - pt);

		if (dur.count() > 500)
		{
			pt = newT;
			std::cout << ".";
		}
	}
	std::cout << std::endl;

	PR::TimePoint renderEnd = std::chrono::high_resolution_clock::now();
	PR_LOGGER.logf(PR::L_Info, PR::M_Scene, "Rendering finished [%f sec]...",
		std::chrono::duration_cast<std::chrono::milliseconds>(renderEnd - renderStart).count() / 1000.0f);

	PR::RenderResult result = renderer.result();
	std::cout << "Rays: " << renderer.rayCount() << std::endl;
	float maxDepth = result.maxDepth();
	scene.clear();

	// TODO: Save png
	PR::RGBConverter converter;

	PP::uint8* imageData = new PP::uint8[3 * result.width() * result.height()];
	PP::uint8* imageData2 = new PP::uint8[3 * result.width() * result.height()];
	for (PP::uint32 y = 0; y < result.height(); ++y)
	{
		for (PP::uint32 x = 0; x < result.width(); ++x)
		{
			size_t index = y * result.width() * 3 + x * 3;
			float r;
			float g;
			float b;

			converter.convert(result.point(x, y), r, g, b);
			r = PM::pm_MinT<float>(1, r);
			g = PM::pm_MinT<float>(1, g);
			b = PM::pm_MinT<float>(1, b);

			imageData[index] = r * 255;
			imageData[index + 1] = g * 255;
			imageData[index + 2] = b * 255;

			float d = result.depth(x, y)/maxDepth;
			d = d < 0 ? 0 : 1-d;

			imageData2[index] = d * 255;
			imageData2[index + 1] = d * 255;
			imageData2[index + 2] = d * 255;
		}
	}

	PP::Image image(imageData, result.width(), result.height(), PP::CF_RGB);
	PP::Image image2(imageData2, result.width(), result.height(), PP::CF_RGB);

	pearpic.write("test.tga", image);
	pearpic.write("test_depth.tga", image2);

	// Release PearPic extensions
	pearpic.exit();

#if defined(PR_OS_WINDOWS) && defined(PR_DEBUG)
	system("pause");
#endif
	return 0;
}