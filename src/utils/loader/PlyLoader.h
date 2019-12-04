#pragma once

#include "SubGraphLoader.h"
#include <map>

namespace PR {
class PR_LIB_UTILS PlyLoader : public SubGraphLoader {
public:
	explicit PlyLoader(const std::string& name);
	~PlyLoader();

	inline float scale() const
	{
		return mScale;
	}

	inline void setScale(float f)
	{
		mScale = f;
	}

	inline void flipNormal(bool b)
	{
		mFlipNormal = b;
	}

	void load(const std::wstring& file, Environment* env) override;

private:
	std::string mName;
	float mScale;
	bool mFlipNormal;
};
} // namespace PR
