#pragma once

#include "PR_Config.h"

namespace PR {
class RenderContext;
class PR_LIB IFreezable {
public:
	IFreezable();
	virtual ~IFreezable() = default;

	/* The object shall not be changed after this. */
	void freeze(RenderContext* context);

	inline bool isFrozen() const { return mFrozen; }

protected:
	virtual void onFreeze(RenderContext* context) = 0;

private:
	bool mFrozen;
};
}
