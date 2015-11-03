#pragma once

namespace PR
{
	class Entity;
	class FacePoint;
	class Ray;
	class Renderer;
	class Material
	{
	public:
		virtual void apply(Ray& in, Entity* entity, const FacePoint& point, Renderer* renderer) = 0;
	};
}