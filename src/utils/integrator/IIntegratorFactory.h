#pragma once

#include "plugin/Plugin.h"

namespace PR {
class RenderContext;
class IIntegrator;
class PR_LIB_UTILS IIntegratorFactory : public IPlugin {
public:
	IIntegratorFactory()		  = default;
	virtual ~IIntegratorFactory() = default;

	virtual std::shared_ptr<IIntegrator> create() = 0;
	virtual const std::vector<std::string>& getNames() const		= 0;

	inline PluginType type() const override { return PT_INTEGRATOR; }
};
} // namespace PR
