#include <sstream>

#include "FileLogListener.h"

#include "scene/Camera.h"
#include "scene/Scene.h"

#include "entity/SphereEntity.h"

#include "renderer/Renderer.h"

// PearPic
#include "PearPic.h"
#include "Image.h"

int main(int argc, char** argv)
{
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

	PR::Camera camera(10, 10, PM::pm_DegToRad(60), PM::pm_DegToRad(60), "Test_Cam");
	PR::Scene scene("Test");
	PR::Renderer renderer(500, 500);

	PR::Entity* e = new PR::SphereEntity("Sphere", 2);
	e->setPosition(PM::pm_Set(0, 0, 5));
	scene.addEntity(e);

	PR::RenderResult result = renderer.render(&camera, &scene);
	scene.clear();

	// TODO: Save png
	PP::uint8* imageData = new PP::uint8[3 * result.width() * result.height()];
	for (PP::uint32 y = 0; y < result.height(); ++y)
	{
		for (PP::uint32 x = 0; x < result.width(); ++x)
		{
			size_t index = y * result.width() * 3 + x * 3;
			imageData[index] = result.point(x, y) * 255;
			imageData[index+1] = result.point(x, y) * 255;
			imageData[index+2] = result.point(x, y) * 255;
		}
	}

	PP::Image image(imageData, 3 * result.width() * result.height(),
		result.width(), result.height(), 8, 3, PP::CF_RGB);
	image.make_external();

	pearpic.write("test.png", image);

	delete[] imageData;

	// Release PearPic extensions
	pearpic.exit();
	return 0;
}