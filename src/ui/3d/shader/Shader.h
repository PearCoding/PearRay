#pragma once
#include "3d/ShadingContext.h"
#include "ShaderProgram.h"

namespace PR {
namespace UI {
enum ShaderFlags {
	SF_HasPositionAttrib = 0x1,
	SF_HasNormalAttrib	 = 0x2,
	SF_HasColorAttrib	 = 0x4,
	SF_HasWeightAttrib	 = 0x8,
	SF_HasTransparency	 = 0x100
};

class PR_LIB_UI Shader {
public:
	Shader();
	Shader(const std::string& vertexShader, const std::string& fragmentShader);
	virtual ~Shader();

	inline bool isValid() const { return mProgram != nullptr; }
	inline int flags() const { return mFlags; }

	inline bool hasTransparency() const { return (flags() & SF_HasTransparency) != 0; }

	int positionAttrib() const { return mPositionAttrib; }
	int normalAttrib() const { return mNormalAttrib; }
	int colorAttrib() const { return mColorAttrib; }
	int weightAttrib() const { return mWeightAttrib; }

	void create();
	void bind(const ShadingContext& sc);

protected:
	virtual void onInit();
	virtual void onBind(const ShadingContext& sc);

	inline ShaderProgram* program() { return mProgram.get(); }
	inline const ShaderProgram* program() const { return mProgram.get(); }

	inline void enableTransparency(bool b)
	{
		if (b)
			setFlags(flags() | SF_HasTransparency);
		else
			setFlags(flags() & ~SF_HasTransparency);
	}
	inline void setFlags(int f) { mFlags = f; }

private:
	void initialize();
	bool mInitialized;

	int mFlags;

	std::unique_ptr<ShaderProgram> mProgram;

	int mPositionAttrib;
	int mNormalAttrib;
	int mColorAttrib;
	int mWeightAttrib;
	int mMatrixUniform;

	std::string mVertexShader;
	std::string mFragmentShader;
};
} // namespace UI
} // namespace PR