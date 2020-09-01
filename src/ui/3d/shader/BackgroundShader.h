#pragma once

#include "Shader.h"

namespace PR {
namespace UI {
// Special purpose shader. No real geometry needed
class PR_LIB_UI BackgroundShader : public Shader {
public:
	BackgroundShader();
	virtual ~BackgroundShader() = default;

	inline void setColorStart(const Vector4f& c) { mColorStart = c; }
	inline const Vector4f& colorStart() const { return mColorStart; }

	inline void setColorEnd(const Vector4f& c) { mColorEnd = c; }
	inline const Vector4f& colorEnd() const { return mColorEnd; }

protected:
	virtual void onInit() override;
	virtual void onBind(const ShadingContext& sc) override;

private:
	int mColorStartUniform;
	int mColorEndUniform;

	Vector4f mColorStart;
	Vector4f mColorEnd;
};
} // namespace UI
} // namespace PR
