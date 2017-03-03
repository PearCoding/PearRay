#include <boost/python.hpp>
#include "Random.h"
#include "sampler/Sampler.h"
#include "sampler/HaltonQMCSampler.h"
#include "sampler/MultiJitteredSampler.h"
#include "sampler/RandomSampler.h"
#include "sampler/StratifiedSampler.h"
#include "sampler/UniformSampler.h"

using namespace PR;
namespace bpy = boost::python;
namespace PRPY
{
    class SamplerWrap : public Sampler, public bpy::wrapper<Sampler>
    {
    public:
        inline float generate1D(uint32 index) override
        {
            return this->get_override("generate1D")(index);
        }

        inline PM::vec2 generate2D(uint32 index) override
        {
            return this->get_override("generate2D")(index);
        }

        inline PM::vec3 generate3D(uint32 index) override
        {
            return this->get_override("generate3D")(index);
        }
    };

    void setup_sampler()
    {
        bpy::class_<SamplerWrap, boost::noncopyable>("Sampler")
            .def("generate1D", bpy::pure_virtual(&Sampler::generate1D))
            .def("generate2D", bpy::pure_virtual(&Sampler::generate2D))
            .def("generate3D", bpy::pure_virtual(&Sampler::generate3D))
        ;

        bpy::class_<HaltonQMCSampler, bpy::bases<Sampler>, boost::noncopyable>
            ("HaltonQMCSampler", bpy::init<uint32, bpy::optional<bool, uint32, uint32, uint32>>())
        ;

        bpy::class_<MultiJitteredSampler, bpy::bases<Sampler>, boost::noncopyable>
            ("MultiJitteredSampler", bpy::init<Random&, uint32>())
        ;

        bpy::class_<RandomSampler, bpy::bases<Sampler>, boost::noncopyable>
            ("RandomSampler", bpy::init<Random&>())
        ;

        bpy::class_<StratifiedSampler, bpy::bases<Sampler>, boost::noncopyable>
            ("StratifiedSampler", bpy::init<Random&, uint32>())
        ;

        bpy::class_<UniformSampler, bpy::bases<Sampler>, boost::noncopyable>
            ("UniformSampler", bpy::init<Random&, uint32>())
        ;

        bpy::class_<Random, boost::noncopyable>("Random", bpy::init<bpy::optional<uint32> >())
            .def("get32", (uint32_t(Random::*)())&Random::get32)
            .def("get32", (uint32_t(Random::*)(uint32_t, uint32_t))&Random::get32)
            .def("get64", (uint64_t(Random::*)())&Random::get64)
            .def("get64", (uint64_t(Random::*)(uint64_t, uint64_t))&Random::get64)
            .def("getFloat", &Random::getFloat)
            .def("getDouble", &Random::getDouble)
            .def("get2D", &Random::get2D)
            .def("get3D", &Random::get3D)
            .def("get4D", &Random::get4D)
        ;
    }
}