#pragma once

#include "PR_Config.h"

#include <unordered_map>
#include <vector>

namespace PR {
template <class T>
class PR_LIB_CORE AbstractDatabase {
public:
	using ObjectList = std::vector<std::shared_ptr<T>>;

	AbstractDatabase()			= default;
	virtual ~AbstractDatabase() = default;

	inline void shrinkToFit() { mObjects.shrink_to_fit(); }

	inline const ObjectList& getAll() const { return mObjects; }
	inline size_t size() const { return mObjects.size(); }

	// Access with ID is always provided

	inline std::shared_ptr<T> get(uint32 id) const { return this->mObjects[id]; }
	inline bool has(uint32 id) const { return id < this->mObjects.size(); }

	inline std::shared_ptr<T> getSafe(uint32 id) const { return has(id) ? get(id) : nullptr; }

protected:
	inline uint32 nextID() const { return (uint32)size(); }

	ObjectList mObjects;
};

/// Special class which only allows anonymous creation
template <class T>
class PR_LIB_CORE AnonymousDatabase : public AbstractDatabase<T> {
public:
	AnonymousDatabase()			 = default;
	virtual ~AnonymousDatabase() = default;

	inline uint32 add(const std::shared_ptr<T>& mat)
	{
		const uint32 id = this->nextID();
		this->mObjects.emplace_back(mat);
		return id;
	}
};

/// Special class which only allows named creation
template <class T>
class PR_LIB_CORE NamedDatabase : public AbstractDatabase<T> {
public:
	using NamedObjectList = std::unordered_map<std::string, uint32>;

	NamedDatabase()			 = default;
	virtual ~NamedDatabase() = default;

	inline uint32 add(const std::string& name, const std::shared_ptr<T>& mat)
	{
		const uint32 id = this->nextID();
		this->mObjects.emplace_back(mat);
		mNamedObjects[name] = id;
		return id;
	}

	using AbstractDatabase<T>::get;
	using AbstractDatabase<T>::has;

	inline uint32 getID(const std::string& name) const { return (mNamedObjects.count(name) != 0) ? mNamedObjects.at(name) : PR_INVALID_ID; }
	inline std::shared_ptr<T> get(const std::string& name) const { return (mNamedObjects.count(name) != 0) ? this->get(mNamedObjects.at(name)) : nullptr; }
	inline bool has(const std::string& name) const { return get(name) != nullptr; }

	inline const NamedObjectList& getAllNamed() const { return mNamedObjects; }

protected:
	NamedObjectList mNamedObjects;
};

/// Special class which allows anonymous and named creation
template <class T>
class PR_LIB_CORE MixedDatabase : public NamedDatabase<T> {
public:
	MixedDatabase()			 = default;
	virtual ~MixedDatabase() = default;

	using NamedDatabase<T>::add;
	inline uint32 add(const std::shared_ptr<T>& mat)
	{
		const uint32 id = this->nextID();
		this->mObjects.emplace_back(mat);
		return id;
	}
};
} // namespace PR
