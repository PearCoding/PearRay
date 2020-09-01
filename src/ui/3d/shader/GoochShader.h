#pragma once

#include "Shader.h"

namespace PR {
namespace UI {
class PR_LIB_UI GoochShader : public Shader {
public:
	GoochShader();
	virtual ~GoochShader() = default;

	inline void setColorWarm(const Vector4f& c)
	{
		mColorWarm = c;
		updateFlags();
	}
	inline const Vector4f& colorWarm() const { return mColorWarm; }

	inline void setColorCold(const Vector4f& c)
	{
		mColorCold = c;
		updateFlags();
	}
	inline const Vector4f& colorCold() const { return mColorCold; }

protected:
	virtual void onInit() override;
	virtual void onBind(const ShadingContext& sc) override;

private:
	void updateFlags();

	int mColorWarmUniform;
	int mColorColdUniform;
	int mNormalMatUniform;

	Vector4f mColorWarm;
	Vector4f mColorCold;
};
} // namespace UI
} // namespace PR
