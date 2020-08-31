#pragma once

#include "Logger.h"

#include <map>
#include <typeinfo>
#include <vector>

namespace PR {
template <class OBJ, class FAC>
class PR_LIB_LOADER AbstractManager {
public:
	using ObjectList = std::vector<std::shared_ptr<OBJ>>;
	using FactoryMap = std::map<std::string, std::shared_ptr<FAC>>;

	AbstractManager()		   = default;
	virtual ~AbstractManager() = default;

	inline uint32 nextID() const { return (uint32)size(); }

	inline uint32 addObject(const std::shared_ptr<OBJ>& mat)
	{
		const uint32 id = nextID();
		mObjects.emplace_back(mat);
		return id;
	}

	inline std::shared_ptr<OBJ> getObject(uint32 id) const { return mObjects[id]; }
	inline bool hasObject(uint32 id) const { return id < mObjects.size(); }
	inline const ObjectList& getAll() const { return mObjects; }
	inline size_t size() const { return mObjects.size(); }

	void addFactory(const std::shared_ptr<FAC>& ptr)
	{
		if (!ptr) {
			PR_LOG(L_ERROR) << "Expected " << typeid(FAC).name() << " but got NULL instead.";
			return;
		}

		auto names = ptr->getNames();
		for (const std::string& alias : names) {
			if (mFactories.count(alias)) {
				PR_LOG(L_WARNING) << "Name " << alias << " already in use! Ignoring it." << std::endl;
				continue;
			}

			mFactories[alias] = ptr;
		}
	}

	inline std::shared_ptr<FAC> getFactory(const std::string& alias) const
	{
		if (mFactories.count(alias) > 0) {
			return mFactories.at(alias);
		} else {
			return nullptr;
		}
	}

	inline const FactoryMap& factoryMap() const { return mFactories; }

protected:
	ObjectList mObjects;
	FactoryMap mFactories;
};
} // namespace PR
