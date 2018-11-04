#pragma once

#include "PR_Config.h"
#include <map>
#include <vector>

namespace PR {
class RenderManager;

template <class OBJ, class FAC>
class PR_LIB_INLINE AbstractManager {
public:
	AbstractManager()		   = default;
	virtual ~AbstractManager() = default;

	inline uint32 nextID() const { return size(); }

	inline uint32 addObject(const std::shared_ptr<OBJ>& mat)
	{
		const uint32 id = nextID();
		mObjects.emplace_back(mat);
		return id;
	}

	inline std::shared_ptr<OBJ> getObject(uint32 id) const { return mObjects[id]; }
	inline const std::vector<std::shared_ptr<OBJ>>& getAll() const { return mObjects; }
	inline size_t size() const { return mObjects.size(); }

	virtual bool loadFactory(const RenderManager& mng,
							 const std::string& base, const std::string& name)
		= 0;

	inline std::shared_ptr<FAC> getFactory(const std::string& alias) const
	{
		if (mFactories.count(alias) > 0) {
			return mFactories.at(alias);
		} else {
			return nullptr;
		}
	}

protected:
	std::vector<std::shared_ptr<OBJ>> mObjects;
	std::map<std::string, std::shared_ptr<FAC>> mFactories;
};
} // namespace PR
