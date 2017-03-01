#include "pymath.h"
#include "material/Material.h"
#include "shader/ShaderClosure.h"
#include "renderer/RenderContext.h"

#include "material/BlinnPhongMaterial.h"
#include "material/CookTorranceMaterial.h"
#include "material/DiffuseMaterial.h"
#include "material/GlassMaterial.h"
#include "material/GridMaterial.h"
#include "material/MirrorMaterial.h"
#include "material/OrenNayarMaterial.h"
#include "material/WardMaterial.h"

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
                bpy::make_function(&Material::emission, bpy::return_value_policy<bpy::copy_const_reference >()),
                &Material::setEmission)
            .add_property("shadeable", &Material::canBeShaded, &Material::enableShading)
            .add_property("shadow", &Material::allowsShadow, &Material::enableShadow)
            .add_property("selfShadow", &Material::allowsSelfShadow, &Material::enableSelfShadow)
            .add_property("cameraVisible", &Material::isCameraVisible, &Material::enableCameraVisibility)
        ;
        bpy::register_ptr_to_python<std::shared_ptr<Material> >();

        bpy::class_<BlinnPhongMaterial, bpy::bases<Material> >("BlinnPhongMaterial", bpy::init<uint32>())
            .add_property("albedo",
                bpy::make_function(&BlinnPhongMaterial::albedo, bpy::return_value_policy<bpy::copy_const_reference >()),
                &BlinnPhongMaterial::setAlbedo)
            .add_property("shininess",
                bpy::make_function(&BlinnPhongMaterial::shininess, bpy::return_value_policy<bpy::copy_const_reference >()),
                &BlinnPhongMaterial::setShininess)
            .add_property("fresnelIndex",
                bpy::make_function(&BlinnPhongMaterial::fresnelIndex, bpy::return_value_policy<bpy::copy_const_reference >()),
                &BlinnPhongMaterial::setFresnelIndex)
        ;
        
        { bpy::scope scope = bpy::class_<CookTorranceMaterial, bpy::bases<Material> >("CookTorranceMaterial", bpy::init<uint32>())
            .add_property("fresnelMode",
                &CookTorranceMaterial::fresnelMode,
                &CookTorranceMaterial::setFresnelMode)
            .add_property("distributionMode",
                &CookTorranceMaterial::distributionMode,
                &CookTorranceMaterial::setDistributionMode)
            .add_property("geometryMode",
                &CookTorranceMaterial::geometryMode,
                &CookTorranceMaterial::setGeometryMode)
            .add_property("albedo",
                bpy::make_function(&CookTorranceMaterial::albedo, bpy::return_value_policy<bpy::copy_const_reference >()),
                &CookTorranceMaterial::setAlbedo)
            .add_property("diffuseRoughness",
                bpy::make_function(&CookTorranceMaterial::diffuseRoughness, bpy::return_value_policy<bpy::copy_const_reference >()),
                &CookTorranceMaterial::setDiffuseRoughness)
            .add_property("specularity",
                bpy::make_function(&CookTorranceMaterial::specularity, bpy::return_value_policy<bpy::copy_const_reference >()),
                &CookTorranceMaterial::setSpecularity)
            .add_property("specularRoughnessX",
                bpy::make_function(&CookTorranceMaterial::specularRoughnessX, bpy::return_value_policy<bpy::copy_const_reference >()),
                &CookTorranceMaterial::setSpecularRoughnessX)
            .add_property("specularRoughnessY",
                bpy::make_function(&CookTorranceMaterial::specularRoughnessY, bpy::return_value_policy<bpy::copy_const_reference >()),
                &CookTorranceMaterial::setSpecularRoughnessY)
            .add_property("ior",
                bpy::make_function(&CookTorranceMaterial::ior, bpy::return_value_policy<bpy::copy_const_reference >()),
                &CookTorranceMaterial::setIOR)
            .add_property("conductorAbsorption",
                bpy::make_function(&CookTorranceMaterial::conductorAbsorption, bpy::return_value_policy<bpy::copy_const_reference >()),
                &CookTorranceMaterial::setConductorAbsorption)
            .add_property("reflectivity",
                bpy::make_function(&CookTorranceMaterial::reflectivity, bpy::return_value_policy<bpy::copy_const_reference >()),
                &CookTorranceMaterial::setReflectivity)
        ;

        bpy::enum_<CookTorranceMaterial::FresnelMode>("FresnelMode")
        .value("Dielectric", CookTorranceMaterial::FM_Dielectric)
        .value("Conductor", CookTorranceMaterial::FM_Conductor)
        ;

        bpy::enum_<CookTorranceMaterial::DistributionMode>("DistributionMode")
        .value("Blinn", CookTorranceMaterial::DM_Blinn)
        .value("Beckmann", CookTorranceMaterial::DM_Beckmann)
        .value("GGX", CookTorranceMaterial::DM_GGX)
        ;

        bpy::enum_<CookTorranceMaterial::GeometryMode>("GeometryMode")
        .value("Implicit", CookTorranceMaterial::GM_Implicit)
        .value("Neumann", CookTorranceMaterial::GM_Neumann)
        .value("CookTorrance", CookTorranceMaterial::GM_CookTorrance)
        .value("Kelemen", CookTorranceMaterial::GM_Kelemen)
        ;
        }// End of scope

        bpy::class_<DiffuseMaterial, bpy::bases<Material> >("DiffuseMaterial", bpy::init<uint32>())
            .add_property("albedo",
                bpy::make_function(&DiffuseMaterial::albedo, bpy::return_value_policy<bpy::copy_const_reference >()),
                &DiffuseMaterial::setAlbedo)
        ;

        bpy::class_<GlassMaterial, bpy::bases<Material> >("GlassMaterial", bpy::init<uint32>())
            .add_property("thin",
                &GlassMaterial::isThin,
                &GlassMaterial::setThin)
            .add_property("specularity",
                bpy::make_function(&GlassMaterial::specularity, bpy::return_value_policy<bpy::copy_const_reference >()),
                &GlassMaterial::setSpecularity)
            .add_property("ior",
                bpy::make_function(&GlassMaterial::ior, bpy::return_value_policy<bpy::copy_const_reference >()),
                &GlassMaterial::setIOR)
        ;

        bpy::class_<GridMaterial, bpy::bases<Material> >("GridMaterial", bpy::init<uint32>())
            .add_property("gridCount",
                &GridMaterial::gridCount,
                &GridMaterial::setGridCount)
            .add_property("tileUV",
                &GridMaterial::tileUV,
                &GridMaterial::setTileUV)
            .add_property("firstMaterial",
                bpy::make_function(&GridMaterial::firstMaterial, bpy::return_value_policy<bpy::copy_const_reference >()),
                &GridMaterial::setFirstMaterial)
            .add_property("secondMaterial",
                bpy::make_function(&GridMaterial::secondMaterial, bpy::return_value_policy<bpy::copy_const_reference >()),
                &GridMaterial::setSecondMaterial)
        ;

        bpy::class_<MirrorMaterial, bpy::bases<Material> >("MirrorMaterial", bpy::init<uint32>())
            .add_property("specularity",
                bpy::make_function(&MirrorMaterial::specularity, bpy::return_value_policy<bpy::copy_const_reference >()),
                &MirrorMaterial::setSpecularity)
            .add_property("ior",
                bpy::make_function(&MirrorMaterial::ior, bpy::return_value_policy<bpy::copy_const_reference >()),
                &MirrorMaterial::setIOR)
        ;

        bpy::class_<OrenNayarMaterial, bpy::bases<Material> >("OrenNayarMaterial", bpy::init<uint32>())
            .add_property("albedo",
                bpy::make_function(&OrenNayarMaterial::albedo, bpy::return_value_policy<bpy::copy_const_reference >()),
                &OrenNayarMaterial::setAlbedo)
            .add_property("roughness",
                bpy::make_function(&OrenNayarMaterial::roughness, bpy::return_value_policy<bpy::copy_const_reference >()),
                &OrenNayarMaterial::setRoughness)
        ;

        bpy::class_<WardMaterial, bpy::bases<Material> >("WardMaterial", bpy::init<uint32>())
            .add_property("albedo",
                bpy::make_function(&WardMaterial::albedo, bpy::return_value_policy<bpy::copy_const_reference >()),
                &WardMaterial::setAlbedo)
            .add_property("roughnessX",
                bpy::make_function(&WardMaterial::roughnessX, bpy::return_value_policy<bpy::copy_const_reference >()),
                &WardMaterial::setRoughnessX)
            .add_property("roughnessY",
                bpy::make_function(&WardMaterial::roughnessY, bpy::return_value_policy<bpy::copy_const_reference >()),
                &WardMaterial::setRoughnessY)
            .add_property("specularity",
                bpy::make_function(&WardMaterial::specularity, bpy::return_value_policy<bpy::copy_const_reference >()),
                &WardMaterial::setSpecularity)
            .add_property("reflectivity",
                bpy::make_function(&WardMaterial::reflectivity, bpy::return_value_policy<bpy::copy_const_reference >()),
                &WardMaterial::setReflectivity)
        ;
    }
}