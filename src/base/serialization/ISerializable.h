#pragma once

#include "PR_Config.h"

namespace PR {
class Serializer;
class PR_LIB_BASE ISerializable {
public:
	virtual void serialize(Serializer& serializer) = 0;
};

} // namespace PR
