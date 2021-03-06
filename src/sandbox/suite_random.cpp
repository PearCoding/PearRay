#include "Logger.h"
#include "Random.h"

#include <OpenImageIO/imageio.h>
#include <filesystem>
#include <sstream>

OIIO_NAMESPACE_USING
using namespace PR;
namespace sf = std::filesystem;

constexpr uint64 ITERATIONS = 10000000;
void rnd_seed(uint64 seed)
{
	uint64 finalSeed = seed != 0 ? seed : (uint64)time(NULL);

	Random random(finalSeed);

	const int xres = 32, yres = 32;
	uint64 data[xres * yres];
	unsigned char pixels[xres * yres];

	std::memset(data, 0, sizeof(uint64) * xres * yres);
	for (uint64 i = 0; i < ITERATIONS; ++i) {
		uint32 x = random.get32(0, xres);
		uint32 y = random.get32(0, yres);

		data[y * xres + x] += 1;
	}

	uint64 max = 0;
	for (uint32 i = 0; i < xres; ++i) {
		for (uint32 j = 0; j < yres; ++j) {
			max = std::max(data[j * xres + i], max);
		}
	}

	if (max == 0) {
		std::cout << "Something really messed up in random1" << std::endl;
		return;
	}

	for (uint32 i = 0; i < xres; ++i) {
		for (uint32 j = 0; j < yres; ++j) {
			pixels[j * xres + i] = std::min<uint32>(255,
													static_cast<uint32>(255 * (data[j * xres + i] / (float)max)));
		}
	}

	std::string path;
	if (seed != 0) {
		std::stringstream stream;
		stream << "results/random1/" << seed << ".png";
		path = stream.str();
	} else {
		std::stringstream stream;
		stream << "results/random1/random_" << finalSeed << ".png";
		path = stream.str();
	}

#if OIIO_PLUGIN_VERSION >= 22
	std::unique_ptr<ImageOutput> out = ImageOutput::create(path);
#else
	ImageOutput* out = ImageOutput::create(path);
#endif
	if (!out) {
		std::cout << "Couldn't save image " << path << std::endl;
		return;
	}

	ImageSpec spec(xres, yres, 1, TypeDesc::UINT8);
	out->open(path, spec);
	out->write_image(TypeDesc::UINT8, pixels);
	out->close();

#if OIIO_PLUGIN_VERSION < 22
	ImageOutput::destroy(out);
#endif
}

void suite_random()
{
	sf::create_directory("results/random");

	rnd_seed(0);
	rnd_seed(42);
	rnd_seed(11);
	rnd_seed(313);
	rnd_seed(193);
	rnd_seed(3452360797);
	rnd_seed(0xdeadbeef);
}