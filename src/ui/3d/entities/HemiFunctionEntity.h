#pragma once

#include "GraphicEntity.h"

namespace PR {
namespace UI {
class PR_LIB_UI HemiFunctionEntity : public GraphicEntity {
public:
	using Function = std::function<float(const Vector3f&)>;

	explicit HemiFunctionEntity(const Function& function, int nradius = 48, int nphi = 128);
	virtual ~HemiFunctionEntity();

	void setRadiusCount(int nradius);
	inline int getRadiusCount() const { return mNRadius; }

	void setPhiCount(int nphi);
	inline int phiCount() const { return mNPhi; }

	void setFunction(const Function& f);
	inline Function function() const { return mFunction; }

	inline void rebuild() { setupBuffer(); }

	void enableNormalization(bool b);
	inline bool hasNormalization() const { return mNormalize; }

private:
	void setupBuffer();

	Function mFunction;
	int mNRadius;
	int mNPhi;

	bool mNormalize;
};
} // namespace UI
} // namespace PR