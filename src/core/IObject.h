#pragma once

#include "PR_Config.h"

namespace PR {
class Scene;
class RenderContext;

/* FIXME: Better use a observer system! */
class PR_LIB_CORE IObject {
public:
	IObject()		   = default;
	virtual ~IObject() = default;

	virtual void beforeSceneBuild()
	{
		//PR_ASSERT(mSetupMode == 0, "beforeSceneBuild already called.");
		//mSetupMode = 1;
	}
	virtual void afterSceneBuild(Scene* /*scene*/)
	{
		//PR_ASSERT(mSetupMode == 1, "afterSceneBuild already called.");
		//mSetupMode = 2;
	}
	virtual void beforeRender(RenderContext* /*ctx*/)
	{
		//PR_ASSERT(mSetupMode == 2 || mSetupMode == 4, "beforeRender called at the wrong time.");
		//mSetupMode = 3;
	}
	virtual void afterRender(RenderContext* /*ctx*/)
	{
		//PR_ASSERT(mSetupMode == 3, "afterRender called at the wrong time.");
		//mSetupMode = 4;
	}

private:
	/* Setup Mode is defined as:
	(0) -> beforeSceneBuild
	(1) -> afterSceneBuild
	(2) -> beforeRender
	(3) -> afterRender -> (2)
	*/
	//int mSetupMode = 0;
};
} // namespace PR
