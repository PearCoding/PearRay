#include "IFreezable.h"

namespace PR {
IFreezable::IFreezable()
	: mFrozen(false)
{
}

void IFreezable::freeze(RenderContext* context)
{
	if (!mFrozen) {
		mFrozen = true;
		this->onFreeze(context);
	}
}
}
