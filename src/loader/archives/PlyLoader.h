#pragma once

#include "SubGraphLoader.h"
#include <map>

namespace PR {
class PR_LIB_LOADER PlyLoader : public SubGraphLoader {
public:
	explicit PlyLoader(const std::string& name);
	~PlyLoader();

	inline void setScale(float f) { mScale = f; }
	inline void flipNormal(bool b) { mFlipNormal = b; }
	inline void setCacheMode(int cm) { mCacheMode = cm; }

	void load(const std::wstring& file, const SceneLoadContext& ctx) override;

private:
	std::string mName;
	float mScale;
	bool mFlipNormal;
	int mCacheMode;
};
} // namespace PR
