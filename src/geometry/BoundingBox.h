#pragma once

#include "PearMath.h"

namespace PR
{
	/**
	 * A axis aligned bounding box (AABB)
	 */
	class BoundingBox
	{
	public:
		PM::vec3 UpperBound;
		PM::vec3 LowerBound;

		BoundingBox();
		BoundingBox(const PM::vec3& upperbound, const PM::vec3& lowerbound);

		float width() const;
		float height() const;
		float depth() const;
		float volume() const;

		bool contains(const PM::vec3& point) const;

		void put(const PM::vec3& point);
		void combine(const BoundingBox& other);

		BoundingBox putted(const PM::vec3& point) const;
		BoundingBox combined(const BoundingBox& other) const;
	};
}