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
            .add_property("projectionMapWeight", &PPMSettings::projectionMapWeight, &PPMSettings::setProjectionMapWeight)
            .add_property("projectionMapQuality", &PPMSettings::projectionMapQuality, &PPMSettings::setProjectionMapQuality)
            .add_property("projectionMapPreferCaustic", &PPMSettings::projectionMapPreferCaustic, &PPMSettings::setProjectionMapPreferCaustic)
        ;
        
        bpy::enum_<SamplerMode>("SamplerMode")
        .value("Random", SM_Random)
        .value("Uniform", SM_Uniform)
        .value("Jitter", SM_Jitter)
        .value("MultiJitter", SM_MultiJitter)
        .value("HaltonQMC", SM_HaltonQMC)
        ;
        
        bpy::enum_<DebugMode>("DebugMode")
        .value("None", DM_None)
        .value("Depth", DM_Depth)
		.value("Normal_Both", DM_Normal_Both)
		.value("Normal_Positive", DM_Normal_Positive)
		.value("Normal_Negative", DM_Normal_Negative)
		.value("Normal_Spherical", DM_Normal_Spherical)
		.value("Tangent_Both", DM_Tangent_Both)
		.value("Tangent_Positive", DM_Tangent_Positive)
		.value("Tangent_Negative", DM_Tangent_Negative)
		.value("Tangent_Spherical", DM_Tangent_Spherical)
		.value("Binormal_Both", DM_Binormal_Both)
		.value("Binormal_Positive", DM_Binormal_Positive)
		.value("Binormal_Negative", DM_Binormal_Negative)
		.value("Binormal_Spherical", DM_Binormal_Spherical)
		.value("UV", DM_UV)
		.value("PDF", DM_PDF)
		.value("Emission", DM_Emission)
		.value("Validity", DM_Validity)
        ;
        
        bpy::enum_<IntegratorMode>("IntegratorMode")
        .value("Direct", IM_Direct)
        .value("BiDirect", IM_BiDirect)
        .value("PPM", IM_PPM)
        ;
        
        bpy::enum_<TileMode>("TileMode")
        .value("Linear", TM_Linear)
        .value("Tile", TM_Tile)
        .value("Spiral", TM_Spiral)
        ;
        
        bpy::enum_<TimeMappingMode>("TimeMappingMode")
        .value("Center", TMM_Center)
        .value("Left", TMM_Left)
        .value("Right", TMM_Right)
        ;
        
        bpy::enum_<PPMGatheringMode>("PPMGatheringMode")
        .value("Sphere", PGM_Sphere)
        .value("Dome", PGM_Dome)
        ;
    }
}