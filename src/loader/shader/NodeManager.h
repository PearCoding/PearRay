#pragma once

#include "INodePlugin.h"
#include "plugin/AbstractManager.h"

namespace PR {
class PR_LIB_LOADER NodeManager : public AbstractManager<INodePlugin> {
public:
	NodeManager();
	virtual ~NodeManager();
};
} // namespace PR
