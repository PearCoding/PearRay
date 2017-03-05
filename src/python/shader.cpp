#include <boost/python.hpp>
#include "material/Material.h"
#include "shader/FaceSample.h"
#include "shader/ShaderClosure.h"
#include "shader/ShaderOutput.h"

// Implementations
#include "shader/ConstScalarOutput.h"
#include "shader/ConstSpectralOutput.h"
#include "shader/ConstVectorOutput.h"
#include "shader/ImageScalarOutput.h"
#include "shader/ImageSpectralOutput.h"
#include "shader/ImageVectorOutput.h"

using namespace PR;
namespace bpy = boost::python;
namespace PRPY
{
    #define ATTR(name) \
        inline PM::vec3 get##name##_Py() const { return name; } \
        inline void set##name##_Py(const PM::vec3& v) { name = v; }
    
    #define PROB_FS(name) \
        .add_property(PR_STRINGIFY(name), &FSWrap::get##name##_Py, &FSWrap::set##name##_Py)

    #define PROB_SC(name) \
        .add_property(PR_STRINGIFY(name), &SCWrap::get##name##_Py, &SCWrap::set##name##_Py)

    class FSWrap : public FaceSample, public bpy::wrapper<FaceSample>
    {
    public:
        FSWrap() : FaceSample() {}
        FSWrap(const boost::reference_wrapper<const FaceSample>::type& other) :
            FaceSample(other) {}

        ATTR(P) ATTR(dPdX) ATTR(dPdY) ATTR(dPdZ)
        ATTR(dPdU) ATTR(dPdV) ATTR(dPdW) ATTR(dPdT)
        ATTR(Ng) ATTR(Nx) ATTR(Ny)
        ATTR(UVW) ATTR(dUVWdX) ATTR(dUVWdY) ATTR(dUVWdZ)

        inline PR::Material* material_Py() const { return Material; }
        inline void setMaterial_Py(PR::Material* m) { Material = m; }
    };

    class SCWrap : public ShaderClosure, public bpy::wrapper<ShaderClosure>
    {
    public:
        SCWrap() : ShaderClosure() {}
        SCWrap(const boost::reference_wrapper<const ShaderClosure>::type& other) :
            ShaderClosure(other) {}

        ATTR(P) ATTR(dPdX) ATTR(dPdY) ATTR(dPdZ)
        ATTR(dPdU) ATTR(dPdV) ATTR(dPdW) ATTR(dPdT)
        ATTR(V) ATTR(dVdX) ATTR(dVdY)
        ATTR(N) ATTR(Ng) ATTR(Nx) ATTR(Ny)
        ATTR(UVW) ATTR(dUVWdX) ATTR(dUVWdY) ATTR(dUVWdZ)

        inline PR::Material* material_Py() const { return Material; }
        inline void setMaterial_Py(PR::Material* m) { Material = m; }
    };

    class ScalarShaderOutputWrap : public ScalarShaderOutput, public bpy::wrapper<ScalarShaderOutput>
    {
    public:
        float eval(const ShaderClosure& point) override
        {
            return this->get_override("eval")(point);
        }
    };

    class SpectralShaderOutputWrap : public SpectralShaderOutput, public bpy::wrapper<SpectralShaderOutput>
    {
    public:
        Spectrum eval(const ShaderClosure& point) override
        {
            return this->get_override("eval")(point);
        }
    };

    class VectorShaderOutputWrap : public VectorShaderOutput, public bpy::wrapper<VectorShaderOutput>
    {
    public:
        PM::vec3 eval(const ShaderClosure& point) override
        {
            return this->get_override("eval")(point);
        }
    };

    void setup_shader()
    {
        bpy::class_<FSWrap>("FaceSample")
        PROB_FS(P) PROB_FS(dPdX) PROB_FS(dPdY) PROB_FS(dPdZ)
        PROB_FS(dPdU) PROB_FS(dPdV) PROB_FS(dPdW) PROB_FS(dPdT)
        PROB_FS(Ng) PROB_FS(Nx) PROB_FS(Ny)
        PROB_FS(UVW) PROB_FS(dUVWdX) PROB_FS(dUVWdY) PROB_FS(dUVWdZ)
        .add_property("Material",
             bpy::make_function(&FSWrap::material_Py, bpy::return_internal_reference<>()),
             &FSWrap::setMaterial_Py)
        ;

        bpy::class_<SCWrap>("ShaderClosure")
        PROB_SC(P) PROB_SC(dPdX) PROB_SC(dPdY) PROB_SC(dPdZ)
        PROB_SC(dPdU) PROB_SC(dPdV) PROB_SC(dPdW) PROB_SC(dPdT)
        PROB_SC(V) PROB_SC(dVdX) PROB_SC(dVdY)
        PROB_SC(N) PROB_SC(Ng) PROB_SC(Nx) PROB_SC(Ny)
        PROB_SC(UVW) PROB_SC(dUVWdX) PROB_SC(dUVWdY) PROB_SC(dUVWdZ)
        .def_readwrite("T", &ShaderClosure::T)
        .def_readwrite("WavelengthIndex", &ShaderClosure::WavelengthIndex)
        .def_readwrite("Depth2", &ShaderClosure::Depth2)
        .def_readwrite("NgdotV", &ShaderClosure::NgdotV)
        .def_readwrite("NdotV", &ShaderClosure::NdotV)
        .def_readwrite("Flags", &ShaderClosure::Flags)
        .def_readwrite("EntityID", &ShaderClosure::EntityID)
        .add_property("Material",
             bpy::make_function(&SCWrap::material_Py, bpy::return_internal_reference<>()),
             &SCWrap::setMaterial_Py)
        ;

        bpy::enum_<ShaderClosureFlags>("ShaderClosureFlags")
        .value("Inside", SCF_Inside)
        ;

        bpy::class_<ScalarShaderOutputWrap, std::shared_ptr<ScalarShaderOutputWrap>, boost::noncopyable>("ScalarShaderOutput")
        .def("eval", bpy::pure_virtual(&ScalarShaderOutput::eval))
        ;

        bpy::class_<SpectralShaderOutputWrap, std::shared_ptr<SpectralShaderOutputWrap>, boost::noncopyable>("SpectralShaderOutput")
        .def("eval", bpy::pure_virtual(&SpectralShaderOutput::eval))
        ;

        bpy::class_<VectorShaderOutputWrap, std::shared_ptr<VectorShaderOutputWrap>, boost::noncopyable>("VectorShaderOutput")
        .def("eval", bpy::pure_virtual(&VectorShaderOutputWrap::eval))
        ;

        bpy::register_ptr_to_python<std::shared_ptr<ScalarShaderOutput> >();
        bpy::register_ptr_to_python<std::shared_ptr<SpectralShaderOutput> >();
        bpy::register_ptr_to_python<std::shared_ptr<VectorShaderOutput> >();

        // Implementations
        bpy::class_<ConstScalarShaderOutput, std::shared_ptr<ConstScalarShaderOutput>, bpy::bases<ScalarShaderOutput>, boost::noncopyable >
            ("ConstScalarShaderOutput", bpy::init<float>())
        ;
        bpy::implicitly_convertible<std::shared_ptr<ConstScalarShaderOutput>, std::shared_ptr<ScalarShaderOutput> >();
        bpy::class_<ConstSpectralShaderOutput, std::shared_ptr<ConstSpectralShaderOutput>, bpy::bases<SpectralShaderOutput>, boost::noncopyable >
            ("ConstSpectralShaderOutput", bpy::init<const PR::Spectrum&>())
        ;
        bpy::implicitly_convertible<std::shared_ptr<ConstSpectralShaderOutput>, std::shared_ptr<SpectralShaderOutput> >();
        bpy::class_<ConstVectorShaderOutput, std::shared_ptr<ConstVectorShaderOutput>, bpy::bases<VectorShaderOutput>, boost::noncopyable >
            ("ConstVectorShaderOutput", bpy::init<const PM::vec3&>())
        ;
        bpy::implicitly_convertible<std::shared_ptr<ConstVectorShaderOutput>, std::shared_ptr<VectorShaderOutput> >();
        // bpy::class_<ImageScalarOutput, std::shared_ptr<ImageScalarOutput>, bpy::bases<ScalarShaderOutput>, boost::noncopyable >
        //     ("ImageScalarOutput", bpy::init<float>())
        // ;
        // bpy::class_<ImageSpectralOutput, std::shared_ptr<ImageSpectralOutput>, bpy::bases<SpectralShaderOutput>, boost::noncopyable >
        //     ("ImageSpectralOutput", bpy::init<const PR::Spectrum&>())
        // ;
        // bpy::class_<ImageVectorOutput, std::shared_ptr<ImageVectorOutput>, bpy::bases<VectorShaderOutput>, boost::noncopyable >
        //     ("ImageVectorOutput", bpy::init<const PM::vec3&>())
        // ;
    }
}