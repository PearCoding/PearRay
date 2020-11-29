#pragma once

#include "Logger.h"

#include <typeinfo>
#include <unordered_map>
#include <vector>

namespace PR {
template <class FAC>
class PR_LIB_LOADER AbstractManager {
public:
	using FactoryMap = std::unordered_map<std::string, std::shared_ptr<FAC>>;

	AbstractManager()		   = default;
	virtual ~AbstractManager() = default;

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
	FactoryMap mFactories;
};
} // namespace PR
