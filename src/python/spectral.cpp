#include <boost/python.hpp>

#include "spectral/Spectrum.h"
#include "spectral/IntensityConverter.h"
#include "spectral/XYZConverter.h"
#include "spectral/RGBConverter.h"

using namespace PR;
namespace bpy = boost::python; 
namespace PRPY
{
    class SpectrumWrap : public Spectrum, public bpy::wrapper<Spectrum>
    {
    public:
        SpectrumWrap() = default;
        SpectrumWrap(const SpectrumWrap& other) = default;
        SpectrumWrap(SpectrumWrap&& other) = default;
        SpectrumWrap& operator=(const SpectrumWrap& other) = default;
        SpectrumWrap& operator=(SpectrumWrap&& other) = default;

        SpectrumWrap(const boost::reference_wrapper<const Spectrum>::type& other) :
            Spectrum(other) {}
        
        explicit SpectrumWrap(float f) : Spectrum(f) {}
        explicit SpectrumWrap(const bpy::list& list) :
            Spectrum()
        {
            fillFromList(list);
        }

        static uint32 WAVELENGTH_START_PY() { return Spectrum::WAVELENGTH_START; }
        static uint32 WAVELENGTH_END_PY() { return Spectrum::WAVELENGTH_END; }
        static uint32 WAVELENGTH_AREA_SIZE_PY() { return Spectrum::WAVELENGTH_AREA_SIZE; }
        static uint32 WAVELENGTH_STEP_PY() { return Spectrum::WAVELENGTH_STEP; }
        static uint32 SAMPLING_COUNT_PY() { return Spectrum::SAMPLING_COUNT; }
        static uint32 ILL_SCALE_PY() { return Spectrum::ILL_SCALE; }
    private:
        void fillFromList(const bpy::list& list)
        {
            const auto len = bpy::len(list);

            if(len != Spectrum::SAMPLING_COUNT)// ERROR?
                return;
            
            for(uint32 i = 0; i < Spectrum::SAMPLING_COUNT; ++i)
                setValue(i, bpy::extract<float>(list[i]));
        }
    };

    class XYZConverterWrap : public XYZConverter, public bpy::wrapper<XYZConverter>
    {
    public:
        static bpy::tuple convert(const Spectrum& spec)
        {
            float x, y;
            XYZConverter::convert(spec, x,y);
            return bpy::make_tuple(x,y);
        }
        static bpy::tuple convertXYZ(const Spectrum& spec)
        {
            float x, y,z;
            XYZConverter::convertXYZ(spec, x,y,z);
            return bpy::make_tuple(x,y,z);
        }
        static bpy::tuple toNorm(float X, float Y, float Z)
        {
            float x, y;
            XYZConverter::toNorm(X,Y,Z, x,y);
            return bpy::make_tuple(x,y);
        }
    };

    class RGBConverterWrap : public RGBConverter, public bpy::wrapper<RGBConverter>
    {
    public:
        static bpy::tuple convert(const Spectrum& spec)
        {
            float x, y, z;
            RGBConverter::convert(spec, x,y,z);
            return bpy::make_tuple(x,y,z);
        }
        
        static bpy::tuple toXYZ(float X, float Y, float Z)
        {
            float x, y, z;
            RGBConverter::toXYZ(X,Y,Z, x,y,z);
            return bpy::make_tuple(x,y,z);
        }
        
        static bpy::tuple fromXYZ(float X, float Y, float Z)
        {
            float x, y, z;
            RGBConverter::fromXYZ(X,Y,Z, x,y,z);
            return bpy::make_tuple(x,y,z);
        }
        
        static bpy::tuple gamma(float X, float Y, float Z)
        {
            float x=X, y=Y, z=Z;
            RGBConverter::gamma(x,y,z);
            return bpy::make_tuple(x,y,z);
        }
    };

    void setup_spectral()
    {
        auto spec = bpy::class_<SpectrumWrap>("Spectrum")
            .def(bpy::init<bpy::list>())
            .def(bpy::init<float>())
            .def("fill", (void (Spectrum::*)(float))&Spectrum::fill)
            .def("fill", (void (Spectrum::*)(uint32,uint32,float))&Spectrum::fill)
            .def("clear", &Spectrum::clear)
            .add_property("max", &Spectrum::max)
            .add_property("min", &Spectrum::min)
            .add_property("avg", &Spectrum::avg)
            .def("normalize", &Spectrum::normalize, bpy::return_internal_reference<>())
            .def("normalized", &Spectrum::normalized)
            .def("clamp", &Spectrum::clamp, bpy::return_internal_reference<>())
            .def("clamped", &Spectrum::clamped)
            .def("lerp", (Spectrum& (Spectrum::*)(const Spectrum&, float))&Spectrum::lerp, bpy::return_internal_reference<>())
            .add_property("lerp", bpy::make_function((Spectrum (*)(const Spectrum&, const Spectrum&, float))&Spectrum::lerp))
            .def("sqrt", &Spectrum::sqrt, bpy::return_internal_reference<>())
            .def("sqrted", &Spectrum::sqrted)
            .def("hasNaN", &Spectrum::hasNaN)
            .def("hasInf", &Spectrum::hasInf)
            .def("hasNegative", &Spectrum::hasNegative)
            .def("isOnlyZero", &Spectrum::isOnlyZero)
            .def("weightPhotometric", &Spectrum::weightPhotometric)
            .add_property("luminousFlux", &Spectrum::luminousFlux)
            .def("fromBlackbody", &Spectrum::fromBlackbody).staticmethod("fromBlackbody")
            .def("__getitem__", &Spectrum::value)
            .def("__setitem__", &Spectrum::setValue)
            .def(bpy::self + bpy::self)
            .def(bpy::self - bpy::self)
            .def(bpy::self * bpy::self)
            .def(bpy::self * float())
            .def(float() * bpy::self)
            .def(bpy::self / bpy::self)
            .def(bpy::self / float())
            .def(float() / bpy::self)
            .def(bpy::self += bpy::self)
            .def(bpy::self -= bpy::self)
            .def(bpy::self *= bpy::self)
            .def(bpy::self *= float())
            .def(bpy::self /= bpy::self)
            .def(bpy::self /= float())
            .def(bpy::self == bpy::self)
            .def(bpy::self != bpy::self)
            .add_static_property("WAVELENGTH_START", &SpectrumWrap::WAVELENGTH_START_PY)
            .add_static_property("WAVELENGTH_END", &SpectrumWrap::WAVELENGTH_END_PY)
            .add_static_property("WAVELENGTH_AREA_SIZE", &SpectrumWrap::WAVELENGTH_AREA_SIZE_PY)
            .add_static_property("WAVELENGTH_STEP", &SpectrumWrap::WAVELENGTH_STEP_PY)
            .add_static_property("SAMPLING_COUNT", &SpectrumWrap::SAMPLING_COUNT_PY)
            .add_static_property("ILL_SCALE", &SpectrumWrap::ILL_SCALE_PY)
        ;

        bpy::class_<IntensityConverter, boost::noncopyable>("IntensityConverter", bpy::no_init)
            .def("convert", &IntensityConverter::convert).staticmethod("convert")
        ;

        bpy::class_<XYZConverterWrap, boost::noncopyable>("XYZConverter", bpy::no_init)
            .def("convert", &XYZConverterWrap::convert).staticmethod("convert")
            .def("convertXYZ", &XYZConverterWrap::convertXYZ).staticmethod("convertXYZ")
            .def("luminance", &XYZConverter::luminance).staticmethod("luminance")
            .def("toNorm", &XYZConverterWrap::toNorm).staticmethod("toNorm")
            .def("toSpec", &XYZConverter::toSpec).staticmethod("toSpec")
        ;

        bpy::class_<RGBConverterWrap, boost::noncopyable>("RGBConverter", bpy::no_init)
            .def("convert", &RGBConverterWrap::convert).staticmethod("convert")
            .def("toXYZ", &RGBConverterWrap::toXYZ).staticmethod("toXYZ")
            .def("fromXYZ", &RGBConverterWrap::fromXYZ).staticmethod("fromXYZ")
            .def("luminance", &RGBConverter::luminance).staticmethod("luminance")
            .def("gamma", &RGBConverterWrap::gamma).staticmethod("gamma")
            .def("toSpec", &RGBConverter::toSpec).staticmethod("toSpec")
        ;
    }
}