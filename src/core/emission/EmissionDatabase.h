#pragma once

#include "AbstractDatabase.h"
#include "IEmission.h"

namespace PR {
using EmissionDatabase = MixedDatabase<IEmission>;
} // namespace PR
