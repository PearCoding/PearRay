#pragma once

#include "PR_Config.h"
#include <vector>

namespace PR {

#define PR_PLUGIN_API_INTERFACE_NAME _pr_exports

enum PluginType {
	PT_MATERIAL = 0,
	PT_ENTITY,
	PT_CAMERA,
	PT_LIGHT,
	PT_INFINITELIGHT,
	PT_INTEGRATOR,
};

class PR_LIB_INLINE IPlugin {
public:
	IPlugin()		   = default;
	virtual ~IPlugin() = default;

	virtual bool init()				= 0;
	virtual PluginType type() const = 0;
};

typedef IPlugin* (*GetFactoryPluginPtr)();

struct PR_LIB_INLINE PluginInterface {
	int APIVersion;
	const char* FileName;
	const char* ClassName;
	const char* PluginName;
	const char* PluginVersion;
	GetFactoryPluginPtr InitFunction;
};
} // namespace PR

#define PR_INTERNAL_PLUGIN_DEFINE_FACTORY(className, entityType, typeEnum)                                 \
	class entityType;                                                                                         \
	class Registry;                                                                                        \
	class PR_LIB_INLINE className : public IPlugin {                                                       \
	public:                                                                                                \
		className()																				= default; \
		virtual ~className()																	= default; \
		virtual std::shared_ptr<entityType> create(uint32 id, uint32 uuid, const Registry& reg) = 0;       \
		virtual const std::vector<std::string>& getNames() const								= 0;       \
		inline PluginType type() const override { return typeEnum; }                                                \
	}

#define PR_PLUGIN_INIT(classType, pluginName, pluginVersion)              \
	extern "C" {                                                          \
	PR_PLUGIN_EXPORT PR::IPlugin* _pr_getFactoryPlugin()                  \
	{                                                                     \
		static classType factory;                                         \
		return &factory;                                                  \
	}                                                                     \
	PR_PLUGIN_EXPORT PR::PluginInterface PR_PLUGIN_API_INTERFACE_NAME = { \
		PR_PLUGIN_API_VERSION,                                            \
		__FILE__,                                                         \
		#classType,                                                       \
		pluginName,                                                       \
		pluginVersion,                                                    \
		_pr_getFactoryPlugin                                              \
	};                                                                    \
	}
