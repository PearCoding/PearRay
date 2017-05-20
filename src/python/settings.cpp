#include <boost/python.hpp>
#include "renderer/RenderSettings.h"
#include "renderer/PPMSettings.h"

using namespace PR;
namespace bpy = boost::python;
namespace PRPY
{
    void setup_settings()
    {
        bpy::class_<RenderSettings>("RenderSettings")
            .add_property("incremental", &RenderSettings::isIncremental, &RenderSettings::setIncremental)
            .add_property("debugMode", &RenderSettings::debugMode, &RenderSettings::setDebugMode)
            .add_property("integratorMode", &RenderSettings::integratorMode, &RenderSettings::setIntegratorMode)
            .add_property("aaSampler", &RenderSettings::aaSampler, &RenderSettings::setAASampler)
            .add_property("maxAASampleCount", &RenderSettings::maxAASampleCount, &RenderSettings::setMaxAASampleCount)
            .add_property("lensSampler", &RenderSettings::lensSampler, &RenderSettings::setLensSampler)
            .add_property("maxLensSampleCount", &RenderSettings::maxLensSampleCount, &RenderSettings::setMaxLensSampleCount)
            .add_property("timeSampler", &RenderSettings::timeSampler, &RenderSettings::setTimeSampler)
            .add_property("maxTimeSampleCount", &RenderSettings::maxTimeSampleCount, &RenderSettings::setMaxTimeSampleCount)
            .add_property("timeMappingMode", &RenderSettings::timeMappingMode, &RenderSettings::setTimeMappingMode)
            .add_property("timeScale", &RenderSettings::timeScale, &RenderSettings::setTimeScale)
            .add_property("spectralSampler", &RenderSettings::spectralSampler, &RenderSettings::setSpectralSampler)
            .add_property("maxSpectralSampleCount", &RenderSettings::maxSpectralSampleCount, &RenderSettings::setMaxSpectralSampleCount)
            .add_property("maxCameraSampleCount", &RenderSettings::maxCameraSampleCount)
            .add_property("maxDiffuseBounces", &RenderSettings::maxDiffuseBounces, &RenderSettings::setMaxDiffuseBounces)
            .add_property("maxRayDepth", &RenderSettings::maxRayDepth, &RenderSettings::setMaxRayDepth)
            .add_property("cropMaxX", &RenderSettings::cropMaxX, &RenderSettings::setCropMaxX)
            .add_property("cropMinX", &RenderSettings::cropMinX, &RenderSettings::setCropMinX)
            .add_property("cropMaxY", &RenderSettings::cropMaxY, &RenderSettings::setCropMaxY)
            .add_property("cropMinY", &RenderSettings::cropMinY, &RenderSettings::setCropMinY)
            .add_property("maxLightSamples", &RenderSettings::maxLightSamples, &RenderSettings::setMaxLightSamples)
            .add_property("ppm", bpy::make_function(
                (PPMSettings& (RenderSettings::*)())&RenderSettings::ppm,
                bpy::return_internal_reference<>()))
            .add_property("tileMode", &RenderSettings::tileMode, &RenderSettings::setTileMode)
        ;

        bpy::class_<PPMSettings>("PPMSettings")
            .add_property("maxPhotonsPerPass", &PPMSettings::maxPhotonsPerPass, &PPMSettings::setMaxPhotonsPerPass)
            .add_property("maxPassCount", &PPMSettings::maxPassCount, &PPMSettings::setMaxPassCount)
            .add_property("maxGatherRadius", &PPMSettings::maxGatherRadius, &PPMSettings::setMaxGatherRadius)
            .add_property("maxGatherCount", &PPMSettings::maxGatherCount, &PPMSettings::setMaxGatherCount)
            .add_property("gatheringMode", &PPMSettings::gatheringMode, &PPMSettings::setGatheringMode)
            .add_property("squeezeWeight", &PPMSettings::squeezeWeight, &PPMSettings::setSqueezeWeight)
            .add_property("contractRatio", &PPMSettings::contractRatio, &PPMSettings::setContractRatio)
        ;
        
        bpy::enum_<SamplerMode>("SamplerMode")
        .value("RANDOM", SM_Random)
        .value("UNIFORM", SM_Uniform)
        .value("JITTER", SM_Jitter)
        .value("MULTIJITTER", SM_MultiJitter)
        .value("HALTONQMC", SM_HaltonQMC)
        ;
        
        bpy::enum_<DebugMode>("DebugMode")
        .value("NONE", DM_None)
        .value("DEPTH", DM_Depth)
		.value("NORMAL_BOTH", DM_Normal_Both)
		.value("NORMAL_POSITIVE", DM_Normal_Positive)
		.value("NORMAL_NEGATIVE", DM_Normal_Negative)
		.value("NORMAL_SPHERICAL", DM_Normal_Spherical)
		.value("TANGENT_BOTH", DM_Tangent_Both)
		.value("TANGENT_POSITIVE", DM_Tangent_Positive)
		.value("TANGENT_NEGATIVE", DM_Tangent_Negative)
		.value("TANGENT_SPHERICAL", DM_Tangent_Spherical)
		.value("BINORMAL_BOTH", DM_Binormal_Both)
		.value("BINORMAL_POSITIVE", DM_Binormal_Positive)
		.value("BINORMAL_NEGATIVE", DM_Binormal_Negative)
		.value("BINORMAL_SPHERICAL", DM_Binormal_Spherical)
		.value("UVW", DM_UVW)
		.value("PDF", DM_PDF)
		.value("EMISSION", DM_Emission)
		.value("VALIDITY", DM_Validity)
        ;
        
        bpy::enum_<IntegratorMode>("IntegratorMode")
        .value("DIRECT", IM_Direct)
        .value("BIDIRECT", IM_BiDirect)
        .value("PPM", IM_PPM)
        ;
        
        bpy::enum_<TileMode>("TileMode")
        .value("LINEAR", TM_Linear)
        .value("TILE", TM_Tile)
        .value("SPIRAL", TM_Spiral)
        ;
        
        bpy::enum_<TimeMappingMode>("TimeMappingMode")
        .value("CENTER", TMM_Center)
        .value("LEFT", TMM_Left)
        .value("RIGHT", TMM_Right)
        ;
        
        bpy::enum_<PPMGatheringMode>("PPMGatheringMode")
        .value("SPHERE", PGM_Sphere)
        .value("DOME", PGM_Dome)
        ;
    }
}