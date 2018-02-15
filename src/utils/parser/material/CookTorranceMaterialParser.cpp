#include "CookTorranceMaterialParser.h"
#include "Environment.h"
#include "SceneLoader.h"

#include "Logger.h"

#include "material/CookTorranceMaterial.h"

#include "DataLisp.h"

namespace PR {
std::shared_ptr<PR::Material> CookTorranceMaterialParser::parse(Environment* env,
																const std::string& obj, const DL::DataGroup& group) const
{
	DL::Data fresnelModeD	  = group.getFromKey("fresnel_mode");
	DL::Data distributionModeD = group.getFromKey("distribution_mode");
	DL::Data geometryModeD	 = group.getFromKey("geometry_mode");

	DL::Data albedoD		= group.getFromKey("albedo");
	DL::Data diffRoughnessD = group.getFromKey("diffuse_roughness");

	DL::Data specularityD	= group.getFromKey("specularity");
	DL::Data specRoughnessD  = group.getFromKey("specular_roughness");
	DL::Data specRoughnessXD = group.getFromKey("specular_roughness_x");
	DL::Data specRoughnessYD = group.getFromKey("specular_roughness_y");

	DL::Data indexD			 = group.getFromKey("index");
	DL::Data condAbsorptionD = group.getFromKey("conductor_absorption");

	DL::Data reflectivityD = group.getFromKey("reflectivity");

	auto diff = std::make_shared<CookTorranceMaterial>(env->materialCount() + 1);

	diff->setAlbedo(SceneLoader::getSpectralOutput(env, albedoD));
	diff->setDiffuseRoughness(SceneLoader::getScalarOutput(env, diffRoughnessD));

	diff->setSpecularity(SceneLoader::getSpectralOutput(env, specularityD));
	diff->setIOR(SceneLoader::getSpectralOutput(env, indexD, true));
	diff->setConductorAbsorption(SceneLoader::getSpectralOutput(env, condAbsorptionD, true));
	diff->setReflectivity(SceneLoader::getScalarOutput(env, reflectivityD));

	if (specRoughnessD.isValid() && !specRoughnessXD.isValid() && !specRoughnessYD.isValid()) {
		auto roughness = SceneLoader::getScalarOutput(env, specRoughnessD);
		diff->setSpecularRoughnessX(roughness);
		diff->setSpecularRoughnessY(roughness);
	} else if (!specRoughnessD.isValid() && specRoughnessXD.isValid() && specRoughnessYD.isValid()) {
		diff->setSpecularRoughnessX(SceneLoader::getScalarOutput(env, specRoughnessXD));
		diff->setSpecularRoughnessY(SceneLoader::getScalarOutput(env, specRoughnessYD));
	} else if (specRoughnessD.isValid() || specRoughnessXD.isValid() || specRoughnessYD.isValid()) {
		PR_LOGGER.logf(L_Warning, M_Scene, "CookTorrance: Isotropic and Anisotropic specular roughness mismatch.");
	}

	if (fresnelModeD.type() == DL::Data::T_String) {
		std::string s = fresnelModeD.getString();
		std::transform(s.begin(), s.end(), s.begin(), ::tolower);
		if (s == "dielectric")
			diff->setFresnelMode(CookTorranceMaterial::FM_Dielectric);
		else if (s == "conductor")
			diff->setFresnelMode(CookTorranceMaterial::FM_Conductor);
		else
			PR_LOGGER.logf(L_Warning, M_Scene, "CookTorrance: Unknown fresnel mode '%s'.", s.c_str());
	}

	if (distributionModeD.type() == DL::Data::T_String) {
		std::string s = distributionModeD.getString();
		std::transform(s.begin(), s.end(), s.begin(), ::tolower);
		if (s == "blinn")
			diff->setDistributionMode(CookTorranceMaterial::DM_Blinn);
		else if (s == "beckmann")
			diff->setDistributionMode(CookTorranceMaterial::DM_Beckmann);
		else if (s == "ggx")
			diff->setDistributionMode(CookTorranceMaterial::DM_GGX);
		else
			PR_LOGGER.logf(L_Warning, M_Scene, "CookTorrance: Unknown distribution mode '%s'.", s.c_str());
	}

	if (geometryModeD.type() == DL::Data::T_String) {
		std::string s = geometryModeD.getString();
		std::transform(s.begin(), s.end(), s.begin(), ::tolower);
		if (s == "implicit")
			diff->setGeometryMode(CookTorranceMaterial::GM_Implicit);
		else if (s == "neumann")
			diff->setGeometryMode(CookTorranceMaterial::GM_Neumann);
		else if (s == "cook_torrance")
			diff->setGeometryMode(CookTorranceMaterial::GM_CookTorrance);
		else if (s == "kelemen")
			diff->setGeometryMode(CookTorranceMaterial::GM_Kelemen);
		else
			PR_LOGGER.logf(L_Warning, M_Scene, "CookTorrance: Unknown geometry mode '%s'.", s.c_str());
	}

	return diff;
}
} // namespace PR
