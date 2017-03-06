#include <boost/python.hpp>

#include "spectral/Spectrum.h"
#include "spectral/ToneMapper.h"

#include "renderer/OutputChannel.h"
#include "SpectralFile.h"

using namespace PR;
namespace bpy = boost::python; 
namespace PRPY
{
    class ToneMapperWrap : public ToneMapper, public bpy::wrapper<ToneMapper>
    {
    public:
        ToneMapperWrap(uint32 w, uint32 h) : ToneMapper(w,h,nullptr) {}
        
        static PyObject* resultToTuple(float* f, uint32 w, uint32 h)
        {
            PyObject* hr = PyTuple_New(h);
            for (uint32 y = 0; y < h; ++y)
            {
                PyObject* wr = PyTuple_New(w);
                for (uint32 x = 0; x < w; ++x)
                {
                    PyObject* cr = PyTuple_New(3);
                    PyTuple_SetItem(cr, 0, PyFloat_FromDouble(f[y*w*3 + x*3 + 0]));
                    PyTuple_SetItem(cr, 1, PyFloat_FromDouble(f[y*w*3 + x*3 + 1]));
                    PyTuple_SetItem(cr, 2, PyFloat_FromDouble(f[y*w*3 + x*3 + 2]));

                    PyTuple_SetItem(wr, x, cr);
                }
                PyTuple_SetItem(hr, y, wr);
            }
            return hr;
        }

        bpy::object map_Py1(const SpectralFile& f) const
        {
            const size_t size = f.width()*f.height()*3;
            float* rgb = new float[size];
            map(f.ptr(), rgb);

            PyObject* obj = resultToTuple(rgb, f.width(), f.height());
            
            delete [] rgb;
            return bpy::object(bpy::handle<>(obj));
        }

        bpy::object map_Py2(const OutputSpectral& f) const
        {
            const size_t size = width()*height()*3;
            float* rgb = new float[size];
            map(f.ptr(), rgb);

            PyObject* obj = resultToTuple(rgb, width(), height());
            
            delete [] rgb;
            return bpy::object(bpy::handle<>(obj));
        }
    };

    void setup_tonemapper()
    {
        bpy::class_<ToneMapperWrap, boost::noncopyable>("ToneMapper", bpy::init<uint32, uint32>())
            .def("map", &ToneMapperWrap::map_Py1)
            .def("map", &ToneMapperWrap::map_Py2)
            .add_property("colorMode", &ToneMapper::colorMode, &ToneMapper::setColorMode)
            .add_property("gammaMode", &ToneMapper::gammaMode, &ToneMapper::setGammaMode)
            .add_property("mapperMode", &ToneMapper::mapperMode, &ToneMapper::setMapperMode)
            .add_property("width", &ToneMapper::width)
            .add_property("height", &ToneMapper::height)
        ;

        bpy::enum_<ToneColorMode>("ToneColorMode")
        .value("sRGB", TCM_SRGB)
        .value("XYZ", TCM_XYZ)
        .value("XYZ_Norm", TCM_XYZ_NORM)
        .value("Luminance", TCM_LUMINANCE)
        ;

        bpy::enum_<ToneGammaMode>("ToneGammaMode")
        .value("None", TGM_None)
        .value("sRGB", TGM_SRGB)
        ;

        bpy::enum_<ToneMapperMode>("ToneMapperMode")
        .value("None", TMM_None)
        .value("Simple_Reinhard", TMM_Simple_Reinhard)
        .value("Clamp", TMM_Clamp)
        .value("Abs", TMM_Abs)
        .value("Positive", TMM_Positive)
        .value("Negative", TMM_Negative)
        .value("Spherical", TMM_Spherical)
        .value("Normalized", TMM_Normalized)
        ;
    }
}