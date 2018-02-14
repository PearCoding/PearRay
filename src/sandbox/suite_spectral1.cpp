#include "Logger.h"

#include "spectral/SpectrumDescriptor.h"
#include "spectral/RGBConverter.h"

#include <OpenImageIO/imageio.h>
#include <boost/filesystem.hpp>
#include <sstream>

OIIO_NAMESPACE_USING
using namespace PR;
namespace bf = boost::filesystem;

static void save_image(const std::string& path,
    float r, float g, float b,
    float r2, float g2, float b2)
{
    const int xres = 32, yres = 32;
    const int channels = 3;
    unsigned char pixels[xres*yres*channels];

    for(uint32 i = 0; i < xres/2; ++i)
    {
        for(uint32 j = 0; j < yres; ++j)
        {
            pixels[j*xres*3 + i*3] = int(r*255);
            pixels[j*xres*3 + i*3 + 1] = int(g*255);
            pixels[j*xres*3 + i*3 + 2] = int(b*255);
        }
    }

    for(uint32 i = xres/2; i < xres; ++i)
    {
        for(uint32 j = 0; j < yres; ++j)
        {
            pixels[j*xres*3 + i*3] = int(r2*255);
            pixels[j*xres*3 + i*3 + 1] = int(g2*255);
            pixels[j*xres*3 + i*3 + 2] = int(b2*255);
        }
    }

    ImageOutput *out = ImageOutput::create (path);
    if (! out)
    {
        std::cout << "Couldn't save image " << path << std::endl;
        return;
    }

    ImageSpec spec(xres, yres, channels, TypeDesc::UINT8);
    out->open(path, spec);
    out->write_image(TypeDesc::UINT8, pixels);
    out->close();

    ImageOutput::destroy(out);
}

static void handle_color(float r, float g, float b)
{
    SpectrumDescriptor desc = SpectrumDescriptor::createStandardSpectral();
    Spectrum spec(&desc);

    PR::RGBConverter::toSpec(spec, r,g,b);
    float R, G, B;
    PR::RGBConverter::convert(spec, R, G, B);

    std::stringstream stream;
    stream << "results/spectral1/" 
        << int(r*255) << "_" << int(g*255) << "_" << int(b*255) << ".png";
    save_image(stream.str(), r,g,b, R,G,B);
}

void suite_spectral1()
{
    bf::create_directory("results/spectral1");

    constexpr float step=0.1;
    for(float r = 0; r <= 1; r+=step)
    {
        for(float g = 0; g <= 1; g+=step)
        {
            for(float b = 0; b <= 1; b+=step)
            {
                handle_color(r,g,b);
            }
        }
    }
}