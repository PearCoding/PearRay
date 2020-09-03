#pragma once

#include "GraphicEntity.h"

namespace PR {
namespace UI {
class PR_LIB_UI PointMapEntity : public GraphicEntity {
public:
	enum MapType {
		MT_Z = 0,
		MT_Spherical
	};

	explicit PointMapEntity(MapType type = MT_Spherical);
	virtual ~PointMapEntity();

	void build(const std::vector<Vector2f>& points, const std::vector<float>& values);

	inline void setMapType(MapType type) { mMapType = type; }
	inline MapType mapType() const { return mMapType; }

	inline void enableColor(bool b = true) { mPopulateColor = b; }
	inline bool isColorEnabled() const { return mPopulateColor; }

private:
	MapType mMapType;
	bool mPopulateColor;
};
} // namespace UI
} // namespace PR