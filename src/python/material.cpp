#include "pymath.h"
#include "material/Material.h"
#include "shader/ShaderClosure.h"
#include "renderer/RenderContext.h"

using namespace PR;
namespace PRPY
{
    class MaterialWrap : public Material, public bpy::wrapper<Material>
    {
    public:
        MaterialWrap(uint32 id) : Material(id) {}

        Spectrum eval(const ShaderClosure& point, const PM::vec3& L, float NdotL) override
        {
            return this->get_override("eval")(point, L, NdotL);
        }

        float pdf(const ShaderClosure& point, const PM::vec3& L, float NdotL) override
        {
            return this->get_override("pdf")(point, L, NdotL);
        }

        PM::vec3 sample(const ShaderClosure& point, const PM::vec3& rnd, float& pdf) override
        {
            bpy::tuple tpl = sample_Py(point, rnd);
            pdf = bpy::extract<float>(tpl[1]);
            return bpy::extract<PM::vec3>(tpl[0]);
        }

        bpy::tuple sample_Py(const ShaderClosure& point, const PM::vec3& rnd)
        {
            return this->get_override("sample")(point, rnd);
        }

        PM::vec3 samplePath(
            const ShaderClosure& point, const PM::vec3& rnd, float& pdf, float& path_weight, uint32 path)
        { 
            bpy::tuple tpl = samplePath_Py(point, rnd, path);
            path_weight = bpy::extract<float>(tpl[2]);
            pdf = bpy::extract<float>(tpl[1]);
            return bpy::extract<PM::vec3>(tpl[0]);
        }

        bpy::tuple samplePath_Py(const ShaderClosure& point, const PM::vec3& rnd, uint32 path)
        {
            if (bpy::override f = this->get_override("samplePath"))
                return f(point, rnd, path);
            return samplePath_PyDef(point, rnd, path);
        }

        bpy::tuple samplePath_PyDef(const ShaderClosure& point, const PM::vec3& rnd, uint32 path)
        {
            float pdf, weight;
            PM::vec3 v = Material::samplePath(point, rnd, pdf, weight, path);
            return bpy::make_tuple(v, pdf, weight);
        }

        uint32 samplePathCount() const override
        { 
            if (bpy::override f = this->get_override("samplePathCount"))
                return f();
            return samplePathCount_PyDef();
        }

        uint32 samplePathCount_PyDef() const
        {
            return Material::samplePathCount();
        }

        void setup(RenderContext* context) override
        { 
            if (bpy::override f = this->get_override("setup"))
                f(context);
            setup_PyDef(context);
        }

        void setup_PyDef(RenderContext* context)
        {
            Material::setup(context);
        }

        std::string dumpInformation() const override
        { 
            if (bpy::override f = this->get_override("dumpInformation"))
                return f();
            return dumpInformation_PyDef();
        }

        std::string dumpInformation_PyDef() const
        {
            return Material::dumpInformation();
        }
    };

    void setup_material()
    {
        bpy::class_<MaterialWrap, boost::noncopyable>("Material", bpy::init<uint32>())
            .def("eval", bpy::pure_virtual(&MaterialWrap::eval))
            .def("pdf", bpy::pure_virtual(&MaterialWrap::pdf))
            .def("sample", bpy::pure_virtual(&MaterialWrap::sample_Py))
            .def("samplePath", &MaterialWrap::samplePath_Py, &MaterialWrap::samplePath_PyDef)
            .def("samplePathCount", &Material::samplePathCount, &MaterialWrap::samplePathCount_PyDef)
            .def("setup", &Material::setup, &MaterialWrap::setup_PyDef)
            .def("dumpInformation", &Material::dumpInformation, &MaterialWrap::dumpInformation_PyDef)
            .add_property("id", &Material::id)
            .add_property("light", &Material::isLight)
            .add_property("emission",
                bpy::make_function(&Material::setEmission, bpy::return_value_policy<bpy::return_internal_reference<> >()),
                bpy::make_function(&Material::setEmission, bpy::return_value_policy<bpy::with_custodian_and_ward<1, 2> >()))
            .add_property("shadeable", &Material::canBeShaded, &Material::enableShading)
            .add_property("shadow", &Material::allowsShadow, &Material::enableShadow)
            .add_property("selfShadow", &Material::allowsSelfShadow, &Material::enableSelfShadow)
            .add_property("cameraVisible", &Material::isCameraVisible, &Material::enableCameraVisibility)
        ;
    }
}