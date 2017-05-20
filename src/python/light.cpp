#include <boost/python.hpp>
#include "material/Material.h"
#include "shader/ShaderClosure.h"
#include "renderer/RenderContext.h"
#include "light/IInfiniteLight.h"
#include "light/DistantLight.h"
#include "light/EnvironmentLight.h"

using namespace PR;
namespace bpy = boost::python;
namespace PRPY
{
    class IInfiniteLightWrap : public IInfiniteLight, public bpy::wrapper<IInfiniteLight>
    {
    public:
        inline Spectrum apply(const Eigen::Vector3f& L) override
        {
            return this->get_override("apply")(L);
        }

        inline Eigen::Vector3f sample(const ShaderClosure& point, const Eigen::Vector3f& rnd, float& pdf) override
        {
            bpy::tuple tpl = bpy::extract<bpy::tuple>(sample_Py(point, rnd));
            pdf = bpy::extract<float>(tpl[1]);
            return bpy::extract<Eigen::Vector3f>(tpl[0]);
        }

        inline bpy::tuple sample_Py(const ShaderClosure& point, const Eigen::Vector3f& rnd)
        {
            return this->get_override("sample")(point, rnd);
        }

        inline void onFreeze() override
        {
            if(bpy::override f = this->get_override("onFreeze"))
                f();
            IInfiniteLight::onFreeze();
        }

        inline void onFreeze_PyDef() { IInfiniteLight::onFreeze(); }
    };
    
    void setup_light()
    {
        bpy::class_<IInfiniteLightWrap, boost::noncopyable>("IInfiniteLight")
            .def("apply", bpy::pure_virtual(&IInfiniteLightWrap::apply))
            .def("sample", bpy::pure_virtual(&IInfiniteLightWrap::sample_Py))
            .def("onFreeze", &IInfiniteLight::onFreeze, &IInfiniteLightWrap::onFreeze_PyDef)
            .def("freeze", &IInfiniteLight::freeze)
            .add_property("frozen", &IInfiniteLight::isFrozen)
        ;
        bpy::register_ptr_to_python<std::shared_ptr<IInfiniteLight> >();

        bpy::class_<DistantLight, bpy::bases<IInfiniteLight>, boost::noncopyable>("DistantLight")
            .add_property("direction", &DistantLight::direction, &DistantLight::setDirection)
            .add_property("material", 
                bpy::make_function(&DistantLight::material, bpy::return_value_policy<bpy::copy_const_reference >()),
                &DistantLight::setMaterial)
        ;

        bpy::class_<EnvironmentLight, bpy::bases<IInfiniteLight>, boost::noncopyable>("EnvironmentLight")
            .add_property("material", 
                bpy::make_function(&EnvironmentLight::material, bpy::return_value_policy<bpy::copy_const_reference >()),
                &EnvironmentLight::setMaterial)
        ;
    }
}