#pragma once

#include "SIMath.h"
#include "entity/Entity.h"

#include <list>

namespace PR {
class Ray;
class RenderContext;

struct CameraSample {
	Eigen::Vector2i Pixel;
	Eigen::Vector2f PixelF;
	Eigen::Vector2f R;
	SI::Time Time;
	uint8 WavelengthIndex;
};

class PR_LIB Camera : public Entity {
public:
	Camera(uint32 id, const std::string& name);
	virtual ~Camera();

	virtual Ray constructRay(RenderContext* context, const CameraSample& sample) const = 0;
};
}
