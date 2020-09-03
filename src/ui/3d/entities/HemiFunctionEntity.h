#pragma once

#include "GraphicEntity.h"

namespace PR {
namespace UI {
class PR_LIB_UI HemiFunctionEntity : public GraphicEntity {
public:
	using Function = std::function<float(float, float)>;

	explicit HemiFunctionEntity(const Function& function, int ntheta = 48, int nphi = 128);
	virtual ~HemiFunctionEntity();

	inline void setThetaCount(int ntheta) { mNTheta = ntheta; }
	inline int getThetaCount() const { return mNTheta; }

	inline void setPhiCount(int nphi) { mNPhi = nphi; }
	inline int phiCount() const { return mNPhi; }

	inline void setFunction(const Function& f) { mFunction = f; }
	inline Function function() const { return mFunction; }

	inline void rebuild() { setupBuffer(); }

private:
	void setupBuffer();

	Function mFunction;
	int mNTheta;
	int mNPhi;
};
} // namespace UI
} // namespace PR