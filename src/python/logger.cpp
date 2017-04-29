#include <boost/python.hpp>
#include <thread>

#include "Logger.h"
#include "LogListener.h"

using namespace PR;
namespace bpy = boost::python; 
namespace PRPY
{
    class LogListenerWrap : public LogListener, public bpy::wrapper<LogListener>
    {
    public:
        void newEntry(Level level, Module m, const std::string& str) override
        {
            this->get_override("newEntry")(level, m, str);
        }
    };

    void setup_logger()
    {
        bpy::class_<LogListenerWrap, boost::noncopyable>("LogListener")
            .def("newEntry", bpy::pure_virtual(&LogListener::newEntry))
        ;

        bpy::class_<Logger, boost::noncopyable>("Logger", bpy::no_init)
        .add_static_property("instance", bpy::make_function(&Logger::instance, 
            bpy::return_value_policy<bpy::reference_existing_object>()))
        .add_property("verbose", &Logger::isVerbose, &Logger::setVerbose)
        .add_property("quiet", &Logger::isQuiet, &Logger::setQuiet)
        .def("log", &Logger::log)
        .def("addListener", &Logger::addListener)
		.def("removeListener", &Logger::removeListener)
        ;

        bpy::enum_<Level>("Level")
        .value("DEBUG", L_Debug)
        .value("INFO", L_Info)
        .value("WARNING", L_Warning)
        .value("ERROR", L_Error)
        .value("FATAL", L_Fatal)
        ;

        bpy::enum_<Module>("Module")
        .value("INTERNAL", M_Internal)
        .value("TEST", M_Test)
        .value("CAMERA", M_Camera)
        .value("GPU", M_GPU)
        .value("ENTITY", M_Entity)
        .value("MATH", M_Math)
        .value("INTEGRATOR", M_Integrator)
        .value("MATERIAL", M_Material)
        .value("SYSTEM", M_System)
        .value("SCENE", M_Scene)
        .value("VOLUME", M_Volume)
        .value("NETWORK", M_Network)
        .value("LOADER", M_Loader)
        .value("SHADER", M_Shader)
        .value("MAIN", M_Main)
        ;
    }
}