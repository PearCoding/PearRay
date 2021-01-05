#pragma once

#include "PluginSpecification.h"

namespace PR {

#define _PR_CONCAT_2(A, B) A##B
#define _PR_CONCAT(A, B) _PR_CONCAT_2(A, B)
#define PR_PLUGIN_API_INTERFACE_NAME_CORE _pr_exports

#ifdef PR_EMBED_PLUGIN
#define PR_PLUGIN_API_INTERFACE_NAME(name) _PR_CONCAT(PR_PLUGIN_API_INTERFACE_NAME_CORE, _##name)
#else
#define PR_PLUGIN_API_INTERFACE_NAME(name) PR_PLUGIN_API_INTERFACE_NAME_CORE
#endif

class PR_LIB_LOADER IPlugin {
public:
	IPlugin()		   = default;
	virtual ~IPlugin() = default;

	virtual bool init()				 = 0;
	virtual const char* type() const = 0;
};

using GetFactoryPluginPtr = IPlugin* (*)();

struct PR_LIB_LOADER PluginInterface {
	int APIVersion;
	const char* FileName;
	const char* ClassName;
	const char* PluginName;
	const char* PluginVersion;
	GetFactoryPluginPtr InitFunction;
};
} // namespace PR

#define PR_INTERNAL_PLUGIN_DEFINE_FACTORY(className, entityType, typeStr)                                                                   \
	class entityType;                                                                                                                       \
	class SceneLoadContext;                                                                                                                 \
	class PR_LIB_LOADER className : public IPlugin {                                                                                        \
	public:                                                                                                                                 \
		using InputSpec																						  = std::vector<ParameterDesc>; \
		className()																							  = default;                    \
		virtual ~className()																				  = default;                    \
		virtual std::shared_ptr<entityType> create(const std::string& type_name, const SceneLoadContext& ctx) = 0;                          \
		virtual const std::vector<std::string>& getNames() const											  = 0;                          \
		virtual PluginSpecification specification(const std::string& /*type_name*/) const					  = 0;                          \
		inline const char* type() const override { return typeStr; }                                                                        \
	}

#define PR_PLUGIN_INIT(classType, pluginName, pluginVersion)                          \
	extern "C" {                                                                      \
	PR::IPlugin* _PR_CONCAT(_pr_getFactoryPlugin, pluginName)()                       \
	{                                                                                 \
		return new classType();                                                       \
	}                                                                                 \
	PR_PLUGIN_EXPORT PR::PluginInterface PR_PLUGIN_API_INTERFACE_NAME(pluginName) = { \
		PR_PLUGIN_API_VERSION,                                                        \
		__FILE__,                                                                     \
		#classType,                                                                   \
		PR_DOUBLEQUOTE(pluginName),                                                   \
		pluginVersion,                                                                \
		_PR_CONCAT(_pr_getFactoryPlugin, pluginName)                                  \
	};                                                                                \
	}
