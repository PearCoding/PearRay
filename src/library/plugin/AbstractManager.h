#pragma once

#include "Logger.h"
#include "PluginManager.h"
#include "renderer/RenderManager.h"

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

	bool loadFactory(const RenderManager& mng,
					 const std::string& base, const std::string& name,
					 const std::string& typeName, PluginType type)
	{
		auto plugin = mng.pluginManager()->load(base + "pr_pl_" + name, *mng.registry());
		if (!plugin) {
			PR_LOG(L_ERROR) << "Could not load plugin of type " << typeName << " and name " << name << " with base path " << base << std::endl;
			return false;
		}

		if (plugin->type() != type) {
			PR_LOG(L_ERROR) << "Plugin " << name << " within base path " << base
							<< " is not plugin of type " << typeName << std::endl;
			return false;
		}

		auto ptrF = std::dynamic_pointer_cast<FAC>(plugin);
		if (!ptrF) {
			PR_LOG(L_ERROR) << "Plugin " << name << " within base path " << base << " is not an integrator plugin even when it says it is!" << std::endl;
			return false;
		}

		auto names = ptrF->getNames();
		for (const std::string& alias : names) {
			if (mFactories.count(alias))
				PR_LOG(L_WARNING) << typeName << " with name " << alias << " already given! Replacing it." << std::endl;

			mFactories[alias] = ptrF;
		}
		return true;
	}

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
