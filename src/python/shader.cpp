#include "pymath.h"
#include "shader/ShaderClosure.h"
#include "shader/ShaderOutput.h"

using namespace PR;
namespace PRPY
{
    #define ATTR_3D(name) \
        inline void set## name ##_Py(const bpy::object& v) { name = to3D(v); } \
		inline bpy::object name ## _Py() const { return convert3D(name); }

    #define ATTR_2D(name) \
        inline void set## name ##_Py(const bpy::object& v) { name = to2D(v); } \
		inline bpy::object name ## _Py() const { return convert2D(name); }

    #define PROB(name) \
        .add_property(PR_STRINGIFY(name), &SCWrap::name##_Py, &SCWrap::set##name##_Py)

    class SCWrap : public ShaderClosure, public bpy::wrapper<ShaderClosure>
    {
    public:
        SCWrap() : ShaderClosure() {}
        SCWrap(const boost::reference_wrapper<const ShaderClosure>::type& other) :
            ShaderClosure(other) {}
        
        ATTR_3D(P)
        ATTR_3D(dPdX)
        ATTR_3D(dPdY)
        ATTR_3D(dPdZ)
        ATTR_3D(dPdU)
        ATTR_3D(dPdV)
        ATTR_3D(dPdT)

        ATTR_3D(V)
        ATTR_3D(dVdX)
        ATTR_3D(dVdY)

        ATTR_3D(N)
        ATTR_3D(Ng)
        ATTR_3D(Nx)
        ATTR_3D(Ny)

        ATTR_2D(UV)
        ATTR_2D(dUVdX)
        ATTR_2D(dUVdY)
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
        PM::vec eval(const ShaderClosure& point) override
        {
            return to3D(this->get_override("eval")(point));
        }

        bpy::object eval_Py(const ShaderClosure& point)
        {
            return convert3D(eval(point));
        }
    };

    void setup_shader()
    {
        bpy::class_<SCWrap>("ShaderClosure")
        PROB(P) PROB(dPdX) PROB(dPdY) PROB(dPdZ)
        PROB(dPdU) PROB(dPdV) PROB(dPdT)
        PROB(V) PROB(dVdX) PROB(dVdY)
        PROB(N) PROB(Ng) PROB(Nx) PROB(Ny)
        PROB(UV) PROB(dUVdX) PROB(dUVdY)
        .def_readwrite("T", &ShaderClosure::T)
        .def_readwrite("WavelengthIndex", &ShaderClosure::WavelengthIndex)
        .def_readwrite("Depth2", &ShaderClosure::Depth2)
        .def_readwrite("NgdotV", &ShaderClosure::NgdotV)
        .def_readwrite("NdotV", &ShaderClosure::NdotV)
        .def_readwrite("Flags", &ShaderClosure::Flags)
        .def_readwrite("EntityID", &ShaderClosure::EntityID)
        //.def_readwrite("Material", &ShaderClosure::Material)
        ;

        bpy::enum_<ShaderClosureFlags>("ShaderClosureFlags")
        .value("Inside", SCF_Inside)
        ;

        bpy::class_<ScalarShaderOutputWrap, boost::noncopyable>("ScalarShaderOutput")
        .def("eval", bpy::pure_virtual(&ScalarShaderOutput::eval))
        ;

        bpy::class_<SpectralShaderOutputWrap, boost::noncopyable>("SpectralShaderOutput")
        .def("eval", bpy::pure_virtual(&SpectralShaderOutput::eval))
        ;

        bpy::class_<VectorShaderOutputWrap, boost::noncopyable>("VectorShaderOutput")
        .def("eval", bpy::pure_virtual(&VectorShaderOutputWrap::eval_Py))
        ;

    }
}