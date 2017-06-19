#include "material/Material.h"
#include "renderer/RenderContext.h"
#include "shader/ShaderClosure.h"
#include <boost/python.hpp>

#include "material/BlinnPhongMaterial.h"
#include "material/CookTorranceMaterial.h"
#include "material/DiffuseMaterial.h"
#include "material/GlassMaterial.h"
#include "material/GridMaterial.h"
#include "material/MirrorMaterial.h"
#include "material/OrenNayarMaterial.h"
#include "material/WardMaterial.h"

#include "npmath.h"

using namespace PR;
namespace bpy = boost::python;
namespace np  = boost::python::numpy;

namespace PRPY {
class MaterialWrap : public Material, public bpy::wrapper<Material> {
public:
	MaterialWrap(uint32 id)
		: Material(id)
	{
	}

	Spectrum eval(const ShaderClosure& point, const Eigen::Vector3f& L, float NdotL) override
	{
		return eval_Py(point, vec3ToPython(L), NdotL);
	}

	Spectrum eval_Py(const ShaderClosure& point, const np::ndarray& L, float NdotL)
	{
		return this->get_override("eval")(point, L, NdotL);
	}

	float pdf(const ShaderClosure& point, const Eigen::Vector3f& L, float NdotL) override
	{
		return pdf_Py(point, vec3ToPython(L), NdotL);
	}

	float pdf_Py(const ShaderClosure& point, const np::ndarray& L, float NdotL)
	{
		return this->get_override("pdf")(point, L, NdotL);
	}

	Eigen::Vector3f sample(const ShaderClosure& point, const Eigen::Vector3f& rnd, float& pdf) override
	{
		bpy::tuple tpl = sample_Py(point, vec3ToPython(rnd));
		pdf			   = bpy::extract<float>(tpl[1]);
		return vec3FromPython(bpy::extract<np::ndarray>(tpl[0]));
	}

	bpy::tuple sample_Py(const ShaderClosure& point, const np::ndarray& rnd)
	{
		return this->get_override("sample")(point, rnd);
	}

	Eigen::Vector3f samplePath(
		const ShaderClosure& point, const Eigen::Vector3f& rnd, float& pdf, float& path_weight, uint32 path)
	{
		bpy::tuple tpl = samplePath_Py(point, vec3ToPython(rnd), path);
		path_weight	= bpy::extract<float>(tpl[2]);
		pdf			   = bpy::extract<float>(tpl[1]);
		return vec3FromPython(bpy::extract<np::ndarray>(tpl[0]));
	}

	bpy::tuple samplePath_Py(const ShaderClosure& point, const np::ndarray& rnd, uint32 path)
	{
		if (bpy::override f = this->get_override("samplePath"))
			return f(point, rnd, path);
		return samplePath_PyDef(point, rnd, path);
	}

	bpy::tuple samplePath_PyDef(const ShaderClosure& point, const np::ndarray& rnd, uint32 path)
	{
		float pdf, weight;
		Eigen::Vector3f v = Material::samplePath(point, vec3FromPython(rnd), pdf, weight, path);
		return bpy::make_tuple(vec3ToPython(v), pdf, weight);
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
	bpy::class_<MaterialWrap, std::shared_ptr<MaterialWrap>, boost::noncopyable>("Material", bpy::init<uint32>())
		.def("eval", bpy::pure_virtual(&MaterialWrap::eval_Py))
		.def("pdf", bpy::pure_virtual(&MaterialWrap::pdf_Py))
		.def("sample", bpy::pure_virtual(&MaterialWrap::sample_Py))
		.def("samplePath", &MaterialWrap::samplePath_Py, &MaterialWrap::samplePath_PyDef)
		.def("samplePathCount", &Material::samplePathCount, &MaterialWrap::samplePathCount_PyDef)
		.def("setup", &Material::setup, &MaterialWrap::setup_PyDef)
		.def("dumpInformation", &Material::dumpInformation, &MaterialWrap::dumpInformation_PyDef)
		.add_property("id", &Material::id)
		.add_property("light", &Material::isLight)
		.add_property("emission",
					  bpy::make_function(&Material::emission, bpy::return_value_policy<bpy::copy_const_reference>()),
					  &Material::setEmission)
		.add_property("shadeable", &Material::canBeShaded, &Material::enableShading)
		.add_property("shadow", &Material::allowsShadow, &Material::enableShadow)
		.add_property("selfShadow", &Material::allowsSelfShadow, &Material::enableSelfShadow)
		.add_property("cameraVisible", &Material::isCameraVisible, &Material::enableCameraVisibility);
	bpy::register_ptr_to_python<std::shared_ptr<Material>>();

	bpy::class_<BlinnPhongMaterial, std::shared_ptr<BlinnPhongMaterial>, bpy::bases<Material>>("BlinnPhongMaterial", bpy::init<uint32>())
		.add_property("albedo",
					  bpy::make_function(&BlinnPhongMaterial::albedo, bpy::return_value_policy<bpy::copy_const_reference>()),
					  &BlinnPhongMaterial::setAlbedo)
		.add_property("shininess",
					  bpy::make_function(&BlinnPhongMaterial::shininess, bpy::return_value_policy<bpy::copy_const_reference>()),
					  &BlinnPhongMaterial::setShininess)
		.add_property("fresnelIndex",
					  bpy::make_function(&BlinnPhongMaterial::fresnelIndex, bpy::return_value_policy<bpy::copy_const_reference>()),
					  &BlinnPhongMaterial::setFresnelIndex);
	bpy::implicitly_convertible<std::shared_ptr<BlinnPhongMaterial>, std::shared_ptr<Material>>();

	{
		bpy::scope scope = bpy::class_<CookTorranceMaterial, std::shared_ptr<CookTorranceMaterial>, bpy::bases<Material>>("CookTorranceMaterial", bpy::init<uint32>())
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
											 bpy::make_function(&CookTorranceMaterial::albedo, bpy::return_value_policy<bpy::copy_const_reference>()),
											 &CookTorranceMaterial::setAlbedo)
							   .add_property("diffuseRoughness",
											 bpy::make_function(&CookTorranceMaterial::diffuseRoughness, bpy::return_value_policy<bpy::copy_const_reference>()),
											 &CookTorranceMaterial::setDiffuseRoughness)
							   .add_property("specularity",
											 bpy::make_function(&CookTorranceMaterial::specularity, bpy::return_value_policy<bpy::copy_const_reference>()),
											 &CookTorranceMaterial::setSpecularity)
							   .add_property("specularRoughnessX",
											 bpy::make_function(&CookTorranceMaterial::specularRoughnessX, bpy::return_value_policy<bpy::copy_const_reference>()),
											 &CookTorranceMaterial::setSpecularRoughnessX)
							   .add_property("specularRoughnessY",
											 bpy::make_function(&CookTorranceMaterial::specularRoughnessY, bpy::return_value_policy<bpy::copy_const_reference>()),
											 &CookTorranceMaterial::setSpecularRoughnessY)
							   .add_property("ior",
											 bpy::make_function(&CookTorranceMaterial::ior, bpy::return_value_policy<bpy::copy_const_reference>()),
											 &CookTorranceMaterial::setIOR)
							   .add_property("conductorAbsorption",
											 bpy::make_function(&CookTorranceMaterial::conductorAbsorption, bpy::return_value_policy<bpy::copy_const_reference>()),
											 &CookTorranceMaterial::setConductorAbsorption)
							   .add_property("reflectivity",
											 bpy::make_function(&CookTorranceMaterial::reflectivity, bpy::return_value_policy<bpy::copy_const_reference>()),
											 &CookTorranceMaterial::setReflectivity);
		bpy::implicitly_convertible<std::shared_ptr<CookTorranceMaterial>, std::shared_ptr<Material>>();

		bpy::enum_<CookTorranceMaterial::FresnelMode>("FresnelMode")
			.value("DIELECTRIC", CookTorranceMaterial::FM_Dielectric)
			.value("CONDUCTOR", CookTorranceMaterial::FM_Conductor);

		bpy::enum_<CookTorranceMaterial::DistributionMode>("DistributionMode")
			.value("BLINN", CookTorranceMaterial::DM_Blinn)
			.value("BECKMANN", CookTorranceMaterial::DM_Beckmann)
			.value("GGX", CookTorranceMaterial::DM_GGX);

		bpy::enum_<CookTorranceMaterial::GeometryMode>("GeometryMode")
			.value("IMPLICIT", CookTorranceMaterial::GM_Implicit)
			.value("NEUMANN", CookTorranceMaterial::GM_Neumann)
			.value("COOKTORRANCE", CookTorranceMaterial::GM_CookTorrance)
			.value("KELEMEN", CookTorranceMaterial::GM_Kelemen);
	} // End of scope

	bpy::class_<DiffuseMaterial, std::shared_ptr<DiffuseMaterial>, bpy::bases<Material>>("DiffuseMaterial", bpy::init<uint32>())
		.add_property("albedo",
					  bpy::make_function(&DiffuseMaterial::albedo, bpy::return_value_policy<bpy::copy_const_reference>()),
					  &DiffuseMaterial::setAlbedo);
	bpy::implicitly_convertible<std::shared_ptr<DiffuseMaterial>, std::shared_ptr<Material>>();

	bpy::class_<GlassMaterial, std::shared_ptr<GlassMaterial>, bpy::bases<Material>>("GlassMaterial", bpy::init<uint32>())
		.add_property("thin",
					  &GlassMaterial::isThin,
					  &GlassMaterial::setThin)
		.add_property("specularity",
					  bpy::make_function(&GlassMaterial::specularity, bpy::return_value_policy<bpy::copy_const_reference>()),
					  &GlassMaterial::setSpecularity)
		.add_property("ior",
					  bpy::make_function(&GlassMaterial::ior, bpy::return_value_policy<bpy::copy_const_reference>()),
					  &GlassMaterial::setIOR);
	bpy::implicitly_convertible<std::shared_ptr<GlassMaterial>, std::shared_ptr<Material>>();

	bpy::class_<GridMaterial, std::shared_ptr<GridMaterial>, bpy::bases<Material>>("GridMaterial", bpy::init<uint32>())
		.add_property("gridCount",
					  &GridMaterial::gridCount,
					  &GridMaterial::setGridCount)
		.add_property("tileUV",
					  &GridMaterial::tileUV,
					  &GridMaterial::setTileUV)
		.add_property("firstMaterial",
					  bpy::make_function(&GridMaterial::firstMaterial, bpy::return_value_policy<bpy::copy_const_reference>()),
					  &GridMaterial::setFirstMaterial)
		.add_property("secondMaterial",
					  bpy::make_function(&GridMaterial::secondMaterial, bpy::return_value_policy<bpy::copy_const_reference>()),
					  &GridMaterial::setSecondMaterial);
	bpy::implicitly_convertible<std::shared_ptr<GridMaterial>, std::shared_ptr<Material>>();

	bpy::class_<MirrorMaterial, std::shared_ptr<MirrorMaterial>, bpy::bases<Material>>("MirrorMaterial", bpy::init<uint32>())
		.add_property("specularity",
					  bpy::make_function(&MirrorMaterial::specularity, bpy::return_value_policy<bpy::copy_const_reference>()),
					  &MirrorMaterial::setSpecularity)
		.add_property("ior",
					  bpy::make_function(&MirrorMaterial::ior, bpy::return_value_policy<bpy::copy_const_reference>()),
					  &MirrorMaterial::setIOR);
	bpy::implicitly_convertible<std::shared_ptr<MirrorMaterial>, std::shared_ptr<Material>>();

	bpy::class_<OrenNayarMaterial, std::shared_ptr<OrenNayarMaterial>, bpy::bases<Material>>("OrenNayarMaterial", bpy::init<uint32>())
		.add_property("albedo",
					  bpy::make_function(&OrenNayarMaterial::albedo, bpy::return_value_policy<bpy::copy_const_reference>()),
					  &OrenNayarMaterial::setAlbedo)
		.add_property("roughness",
					  bpy::make_function(&OrenNayarMaterial::roughness, bpy::return_value_policy<bpy::copy_const_reference>()),
					  &OrenNayarMaterial::setRoughness);
	bpy::implicitly_convertible<std::shared_ptr<OrenNayarMaterial>, std::shared_ptr<Material>>();

	bpy::class_<WardMaterial, std::shared_ptr<WardMaterial>, bpy::bases<Material>>("WardMaterial", bpy::init<uint32>())
		.add_property("albedo",
					  bpy::make_function(&WardMaterial::albedo, bpy::return_value_policy<bpy::copy_const_reference>()),
					  &WardMaterial::setAlbedo)
		.add_property("roughnessX",
					  bpy::make_function(&WardMaterial::roughnessX, bpy::return_value_policy<bpy::copy_const_reference>()),
					  &WardMaterial::setRoughnessX)
		.add_property("roughnessY",
					  bpy::make_function(&WardMaterial::roughnessY, bpy::return_value_policy<bpy::copy_const_reference>()),
					  &WardMaterial::setRoughnessY)
		.add_property("specularity",
					  bpy::make_function(&WardMaterial::specularity, bpy::return_value_policy<bpy::copy_const_reference>()),
					  &WardMaterial::setSpecularity)
		.add_property("reflectivity",
					  bpy::make_function(&WardMaterial::reflectivity, bpy::return_value_policy<bpy::copy_const_reference>()),
					  &WardMaterial::setReflectivity);
	bpy::implicitly_convertible<std::shared_ptr<WardMaterial>, std::shared_ptr<Material>>();
}
}