#pragma once

#include "ray/Ray.h"

namespace PR
{
	class Entity;
	class Renderer;
	class Material
	{
	public:
		virtual void apply(Ray& in, Entity* entity, const PM::vec3& point, const PM::vec3& normal, Renderer* renderer) = 0;
	};
}