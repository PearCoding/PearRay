#pragma once

#include "Affector.h"

namespace PR
{
	class PR_LIB LightAffector : public Affector
	{
	public:
		LightAffector();

		void init(Renderer* renderer) override;
		Spectrum apply(const Ray& in, RenderEntity* entity, const FacePoint& point, RenderContext* context) override;
	};
}