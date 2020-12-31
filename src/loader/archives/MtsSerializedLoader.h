#pragma once

#include "SubGraphLoader.h"

namespace PR {
/// Loaded meshes in Mitsuba Serialized format
class PR_LIB_LOADER MtsSerializedLoader : public SubGraphLoader {
public:
	explicit MtsSerializedLoader(const std::string& name);
	~MtsSerializedLoader();

	inline void setScale(float f) { mScale = f; }
	inline void flipNormal(bool b) { mFlipNormal = b; }

	void load(const std::filesystem::path& file, SceneLoadContext& ctx) override;

private:
	std::string mName;
	float mScale;
	bool mFlipNormal;
};
} // namespace PR
