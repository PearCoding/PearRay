#pragma once

#include "Shader.h"

namespace PR {
namespace UI {
class PR_LIB_UI ColorShader : public Shader {
public:
	ColorShader();
	explicit ColorShader(const Vector4f& c);
	virtual ~ColorShader() = default;

	inline void setColor(const Vector4f& c)
	{
		mColor = c;
		updateFlags();
	}
	inline const Vector4f& color() const { return mColor; }

	inline void setBlendFactor(float f) { mBlendFactor = f; }
	inline float blendFactor() const { return mBlendFactor; }

protected:
	virtual void onInit() override;
	virtual void onBind(const ShadingContext& sc) override;

private:
	void updateFlags();

	int mColorUniform;
	Vector4f mColor;

	int mBlendUniform;
	float mBlendFactor;
};
} // namespace UI
} // namespace PR
