#pragma once

#include "GraphicEntity.h"

namespace PR {
namespace UI {
class PR_LIB_UI GridEntity : public GraphicEntity {
public:
	GridEntity(int count = 10, float wx = 10.0f, float wy = 10.0f);
	virtual ~GridEntity();

	void setGridSize(float wx, float wy);
	inline std::pair<float, float> gridSize() const { return mGridSize; }

	void setGridCount(int count);
	inline int gridCount() const { return mGridCount; }

private:
	void setupBuffer();

	std::pair<float, float> mGridSize;
	int mGridCount;
};
} // namespace UI
} // namespace PR