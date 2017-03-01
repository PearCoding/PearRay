#include <boost/python.hpp>

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
        .value("Debug", L_Debug)
        .value("Info", L_Info)
        .value("Warning", L_Warning)
        .value("Error", L_Error)
        .value("Fatal", L_Fatal)
        ;

        bpy::enum_<Module>("Module")
        .value("Internal", M_Internal)
        .value("Test", M_Test)
        .value("Camera", M_Camera)
        .value("GPU", M_GPU)
        .value("Entity", M_Entity)
        .value("Math", M_Math)
        .value("Integrator", M_Integrator)
        .value("Material", M_Material)
        .value("System", M_System)
        .value("Scene", M_Scene)
        .value("Volume", M_Volume)
        .value("Network", M_Network)
        .value("Loader", M_Loader)
        .value("Shader", M_Shader)
        .value("Main", M_Main)
        ;
    }
}