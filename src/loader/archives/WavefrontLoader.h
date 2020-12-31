#pragma once

#include "SubGraphLoader.h"

namespace PR {
class PR_LIB_LOADER WavefrontLoader : public SubGraphLoader {
public:
	explicit WavefrontLoader(const std::string& override_name);
	~WavefrontLoader();

	inline void setScale(float f) { mScale = f; }
	inline void flipNormal(bool b) { mFlipNormal = b; }

	void load(const std::filesystem::path& file, SceneLoadContext& ctx) override;

private:
	std::string mName;
	float mScale;
	bool mFlipNormal;
};
} // namespace PR
