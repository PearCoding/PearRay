#pragma once

#include "SubGraphLoader.h"

namespace PR {
class PR_LIB_LOADER PlyLoader : public SubGraphLoader {
public:
	explicit PlyLoader(const std::string& name);
	~PlyLoader();

	inline void setScale(float f) { mScale = f; }
	inline void flipNormal(bool b) { mFlipNormal = b; }

	void load(const std::filesystem::path& file, SceneLoadContext& ctx) override;

private:
	std::string mName;
	float mScale;
	bool mFlipNormal;
};
} // namespace PR
