#include "material/Material.h"
#include "renderer/RenderContext.h"
#include "shader/ShaderClosure.h"

#include "material/BlinnPhongMaterial.h"
#include "material/CookTorranceMaterial.h"
#include "material/DiffuseMaterial.h"
#include "material/GlassMaterial.h"
#include "material/GridMaterial.h"
#include "material/MirrorMaterial.h"
#include "material/OrenNayarMaterial.h"
#include "material/WardMaterial.h"

#include "pypearray.h"

using namespace PR;
namespace PRPY {
class MaterialWrap : public Material {
public:
	MaterialWrap(uint32 id)
		: Material(id)
	{
	}

	Spectrum eval(const ShaderClosure& point, const Eigen::Vector3f& L, float NdotL) override
	{
		PYBIND11_OVERLOAD_PURE(Spectrum, Material, eval, point, L, NdotL);
	}

	float pdf(const ShaderClosure& point, const Eigen::Vector3f& L, float NdotL) override
	{
		PYBIND11_OVERLOAD_PURE(float, Material, pdf, point, L, NdotL);
	}

	MaterialSample sample(const ShaderClosure& point, const Eigen::Vector3f& rnd) override
	{
		PYBIND11_OVERLOAD_PURE(MaterialSample, Material, sample, point, rnd);
	}

	MaterialSample samplePath(
		const ShaderClosure& point, const Eigen::Vector3f& rnd, uint32 path)
	{
		PYBIND11_OVERLOAD(MaterialSample, Material, samplePath, point, rnd, path);
	}

	uint32 samplePathCount() const override
	{
		PYBIND11_OVERLOAD(uint32, Material, samplePathCount);
	}

	void setup(RenderContext* context) override
	{
		PYBIND11_OVERLOAD(void, Material, setup, context);
	}

	std::string dumpInformation() const override
	{
		PYBIND11_OVERLOAD(std::string, Material, dumpInformation);
	}
};

void setup_material(py::module& m)
{
	py::class_<MaterialSample>(m, "MaterialSample")
		.def_readwrite("PDF_S", &MaterialSample::PDF_S)
		.def_readwrite("Weight", &MaterialSample::Weight)
		.def_readwrite("L", &MaterialSample::L);

	py::class_<Material, std::shared_ptr<Material>>(m, "Material")
		.def("eval", &Material::eval)
		.def("pdf", &Material::pdf)
		.def("sample", &Material::sample)
		.def("samplePath", &Material::samplePath)
		.def("samplePathCount", &Material::samplePathCount)
		.def("setup", &Material::setup)
		.def("dumpInformation", &Material::dumpInformation)
		.def_property_readonly("id", &Material::id)
		.def_property_readonly("light", &Material::isLight)
		.def_property("emission",
					  &Material::emission,
					  &Material::setEmission)
		.def_property("shadeable", &Material::canBeShaded, &Material::enableShading)
		.def_property("shadow", &Material::allowsShadow, &Material::enableShadow)
		.def_property("selfShadow", &Material::allowsSelfShadow, &Material::enableSelfShadow)
		.def_property("cameraVisible", &Material::isCameraVisible, &Material::enableCameraVisibility);

	py::class_<BlinnPhongMaterial, Material, std::shared_ptr<BlinnPhongMaterial>>(m, "BlinnPhongMaterial")
		.def(py::init<uint32>())
		.def_property("albedo",
					  &BlinnPhongMaterial::albedo,
					  &BlinnPhongMaterial::setAlbedo)
		.def_property("shininess",
					  &BlinnPhongMaterial::shininess,
					  &BlinnPhongMaterial::setShininess)
		.def_property("fresnelIndex",
					  &BlinnPhongMaterial::fresnelIndex,
					  &BlinnPhongMaterial::setFresnelIndex);

	auto scope = py::class_<CookTorranceMaterial, Material, std::shared_ptr<CookTorranceMaterial>>(m, "CookTorranceMaterial")
					 .def(py::init<uint32>())
					 .def_property("fresnelMode",
								   &CookTorranceMaterial::fresnelMode,
								   &CookTorranceMaterial::setFresnelMode)
					 .def_property("distributionMode",
								   &CookTorranceMaterial::distributionMode,
								   &CookTorranceMaterial::setDistributionMode)
					 .def_property("geometryMode",
								   &CookTorranceMaterial::geometryMode,
								   &CookTorranceMaterial::setGeometryMode)
					 .def_property("albedo",
								   &CookTorranceMaterial::albedo,
								   &CookTorranceMaterial::setAlbedo)
					 .def_property("diffuseRoughness",
								   &CookTorranceMaterial::diffuseRoughness,
								   &CookTorranceMaterial::setDiffuseRoughness)
					 .def_property("specularity",
								   &CookTorranceMaterial::specularity,
								   &CookTorranceMaterial::setSpecularity)
					 .def_property("specularRoughnessX",
								   &CookTorranceMaterial::specularRoughnessX,
								   &CookTorranceMaterial::setSpecularRoughnessX)
					 .def_property("specularRoughnessY",
								   &CookTorranceMaterial::specularRoughnessY,
								   &CookTorranceMaterial::setSpecularRoughnessY)
					 .def_property("ior",
								   &CookTorranceMaterial::ior,
								   &CookTorranceMaterial::setIOR)
					 .def_property("conductorAbsorption",
								   &CookTorranceMaterial::conductorAbsorption,
								   &CookTorranceMaterial::setConductorAbsorption)
					 .def_property("reflectivity",
								   &CookTorranceMaterial::reflectivity,
								   &CookTorranceMaterial::setReflectivity);

	py::enum_<CookTorranceMaterial::FresnelMode>(scope, "FresnelMode")
		.value("DIELECTRIC", CookTorranceMaterial::FM_Dielectric)
		.value("CONDUCTOR", CookTorranceMaterial::FM_Conductor);

	py::enum_<CookTorranceMaterial::DistributionMode>(scope, "DistributionMode")
		.value("BLINN", CookTorranceMaterial::DM_Blinn)
		.value("BECKMANN", CookTorranceMaterial::DM_Beckmann)
		.value("GGX", CookTorranceMaterial::DM_GGX);

	py::enum_<CookTorranceMaterial::GeometryMode>(scope, "GeometryMode")
		.value("IMPLICIT", CookTorranceMaterial::GM_Implicit)
		.value("NEUMANN", CookTorranceMaterial::GM_Neumann)
		.value("COOKTORRANCE", CookTorranceMaterial::GM_CookTorrance)
		.value("KELEMEN", CookTorranceMaterial::GM_Kelemen);

	py::class_<DiffuseMaterial, Material, std::shared_ptr<DiffuseMaterial>>(m, "DiffuseMaterial")
		.def(py::init<uint32>())
		.def_property("albedo",
					  &DiffuseMaterial::albedo,
					  &DiffuseMaterial::setAlbedo);

	py::class_<GlassMaterial, Material, std::shared_ptr<GlassMaterial>>(m, "GlassMaterial")
		.def(py::init<uint32>())
		.def_property("thin",
					  &GlassMaterial::isThin,
					  &GlassMaterial::setThin)
		.def_property("specularity",
					  &GlassMaterial::specularity,
					  &GlassMaterial::setSpecularity)
		.def_property("ior",
					  &GlassMaterial::ior,
					  &GlassMaterial::setIOR);

	py::class_<GridMaterial, Material, std::shared_ptr<GridMaterial>>(m, "GridMaterial")
		.def(py::init<uint32>())
		.def_property("gridCount",
					  &GridMaterial::gridCount,
					  &GridMaterial::setGridCount)
		.def_property("tileUV",
					  &GridMaterial::tileUV,
					  &GridMaterial::setTileUV)
		.def_property("firstMaterial",
					  &GridMaterial::firstMaterial,
					  &GridMaterial::setFirstMaterial)
		.def_property("secondMaterial",
					  &GridMaterial::secondMaterial,
					  &GridMaterial::setSecondMaterial);

	py::class_<MirrorMaterial, Material, std::shared_ptr<MirrorMaterial>>(m, "MirrorMaterial")
		.def(py::init<uint32>())
		.def_property("specularity",
					  &MirrorMaterial::specularity,
					  &MirrorMaterial::setSpecularity)
		.def_property("ior",
					  &MirrorMaterial::ior,
					  &MirrorMaterial::setIOR);

	py::class_<OrenNayarMaterial, Material, std::shared_ptr<OrenNayarMaterial>>(m, "OrenNayarMaterial")
		.def(py::init<uint32>())
		.def_property("albedo",
					  &OrenNayarMaterial::albedo,
					  &OrenNayarMaterial::setAlbedo)
		.def_property("roughness",
					  &OrenNayarMaterial::roughness,
					  &OrenNayarMaterial::setRoughness);

	py::class_<WardMaterial, Material, std::shared_ptr<WardMaterial>>(m, "WardMaterial")
		.def(py::init<uint32>())
		.def_property("albedo",
					  &WardMaterial::albedo,
					  &WardMaterial::setAlbedo)
		.def_property("roughnessX",
					  &WardMaterial::roughnessX,
					  &WardMaterial::setRoughnessX)
		.def_property("roughnessY",
					  &WardMaterial::roughnessY,
					  &WardMaterial::setRoughnessY)
		.def_property("specularity",
					  &WardMaterial::specularity,
					  &WardMaterial::setSpecularity)
		.def_property("reflectivity",
					  &WardMaterial::reflectivity,
					  &WardMaterial::setReflectivity);
}
}