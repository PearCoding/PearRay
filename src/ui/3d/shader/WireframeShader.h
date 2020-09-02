#pragma once

#include "Shader.h"

namespace PR {
namespace UI {
class PR_LIB_UI WireframeShader : public Shader {
public:
	WireframeShader(const Vector4f& c = Vector4f::Ones());
	virtual ~WireframeShader() = default;

	inline void setColor(const Vector4f& c)
	{
		mColor = c;
		updateFlags();
	}
	inline const Vector4f& color() const { return mColor; }

protected:
	virtual void onInit() override;
	virtual void onBind(const ShadingContext& sc) override;

private:
	void updateFlags();

	int mColorUniform;
	Vector4f mColor;
};
} // namespace UI
} // namespace PR
