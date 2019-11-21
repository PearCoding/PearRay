#pragma once

#include "PR_Config.h"

namespace PR {
class Scene;
class RenderContext;
class PR_LIB IObject {
public:
	IObject()		   = default;
	virtual ~IObject() = default;

	virtual void beforeSceneBuild() {}
	virtual void afterSceneBuild(Scene* /*scene*/) {}
	virtual void beforeRender(RenderContext* /*ctx*/) {}
	virtual void afterRender(RenderContext* /*ctx*/) {}
};
} // namespace PR
