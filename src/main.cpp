#include <sstream>

#include "FileLogListener.h"

#include "scene/Camera.h"
#include "scene/Scene.h"

#include "entity/SphereEntity.h"

#include "renderer/Renderer.h"

// PearPic
#include "ExtensionManager.h"
#include "Image.h"
#include "Writer.h"

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
	PP::ExtensionManager extensionManager;
	extensionManager.init();

	PR::Camera camera(500, 500, PM::pm_DegToRad(60), PM::pm_DegToRad(60));
	PR::Scene scene("Test");
	PR::Renderer renderer;

	scene.addEntity(new PR::SphereEntity("Sphere", 10));

	PR::RenderResult result = renderer.render(&camera, &scene);
	scene.clear();

	// TODO: Save png
	PP::uint8* imageData = new PP::uint8[3 * camera.width() * camera.height()];
	for (PP::uint32 y = 0; y < camera.height(); ++y)
	{
		for (PP::uint32 x = 0; x < camera.width(); ++x)
		{
			size_t index = y * camera.width() * 3 + x * 3;
			imageData[index] = result.point(x, y) * 255;
			imageData[index+1] = result.point(x, y) * 255;
			imageData[index+2] = result.point(x, y) * 255;
		}
	}
	result.release();

	PP::Image image(imageData, 3 * camera.width() * camera.height(),
		camera.width(), camera.height(), 8, 3, PP::CF_RGB);
	PP::Writer::write(&extensionManager, "test.png", image);

	// Release PearPic extensions
	extensionManager.exit();
	return 0;
}