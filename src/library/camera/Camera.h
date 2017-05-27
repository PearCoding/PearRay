#pragma once

#include "entity/Entity.h"

#include <list>

namespace PR {
class Ray;
class PR_LIB Camera : public Entity {
public:
	Camera(uint32 id, const std::string& name);
	virtual ~Camera();

	/**
		 * nx and ny are screen coordinates [-1 1]
		 */
	virtual Ray constructRay(float nx, float ny, float rx, float ry, float t, uint8 wavelength) const = 0;
};
}
