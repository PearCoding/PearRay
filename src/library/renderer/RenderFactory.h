#pragma once

#include "PR_Config.h"

#include <string>

namespace PR {
class RenderContext;
class RenderManager;
class PR_LIB RenderFactory {
public:
	RenderFactory(const RenderManager* manager);
	virtual ~RenderFactory();

	// index = image index; should be less then itx*ity!
	// itx = image tile count x
	// ity = image tile count y
	std::shared_ptr<RenderContext> create(uint32 index, uint32 itx, uint32 ity) const;
	inline std::shared_ptr<RenderContext> create() const { return create(0, 1, 1); }

	inline const RenderManager* renderManager() const { return mRenderManager; }

private:
	const RenderManager* mRenderManager;
};
} // namespace PR
