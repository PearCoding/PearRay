#pragma once

namespace PR {
inline PerformanceEntry::PerformanceEntry(size_t hash, PerformanceEntry* parent,
										  int line, const char* file, const char* function)
	: mHash(hash)
	, mParent(parent)
	, mLine(line)
	, mFile(file)
	, mFunction(function)
	, mStart()
	, mTicks(0)
{
}

inline size_t PerformanceEntry::hash() const
{
	return mHash;
}

inline PerformanceEntry* PerformanceEntry::parent() const
{
	return mParent;
}

inline int PerformanceEntry::line() const
{
	return mLine;
}

inline const char* PerformanceEntry::file() const
{
	return mFile;
}

inline const char* PerformanceEntry::function() const
{
	return mFunction;
}

inline uint64 PerformanceEntry::ticks() const
{
	return mTicks;
}

inline void PerformanceEntry::start()
{
	mStart = PerformanceEntry::Clock::now();
}

inline void PerformanceEntry::end()
{
	mTicks += std::chrono::duration_cast<std::chrono::microseconds>(PerformanceEntry::Clock::now() - mStart).count();
}

inline std::list<PerformanceEntry*>& PerformanceEntry::children()
{
	return mChildren;
}

inline const std::list<PerformanceEntry*>& PerformanceEntry::children() const
{
	return mChildren;
}

inline PerformanceManager::PerformanceManager()
{
}

inline PerformanceManager::~PerformanceManager()
{
	for (auto p : mEntries)
		for (const auto& e : p.second)
			delete e.Entry;
}

inline PerformanceEntry* PerformanceManager::add(int line, const char* file, const char* function)
{
	auto this_id = std::this_thread::get_id();

	mMutex.lock();
	PerformanceEntry* parent = nullptr;

	if (!mEntries.count(this_id))
		mEntries[this_id] = Set(10, [](const SetEntry& e) { return e.Hash; });
	else
		parent = mCurrentParent;

	Set& set = mEntries[this_id];

	std::stringstream stream;
	stream << function << "$" << file << "$" << line << "$" << this_id;
	if (parent)
		stream << "$" << parent->hash();

	size_t hash = std::hash<std::string>()(stream.str());
	SetEntry hashE(hash);

	PerformanceEntry* entry = nullptr;
	const auto it			= set.find(hashE);
	if (it != set.end()) {
		entry = it->Entry;
	} else {
		entry = new PerformanceEntry(hash, parent, line, file, function);
		PR_ASSERT(entry, "new entry has to be non null");

		hashE.Entry = entry;
		set.insert(hashE);
	}
	mMutex.unlock();

	mCurrentParent = entry;

	if (parent) {
		bool has = false;
		for (auto e : parent->children()) {
			if (e->hash() == hash) {
				has = true;
				break;
			}
		}

		if (!has)
			parent->children().push_back(entry);
	}

	entry->start();
	return entry;
}

inline void PerformanceManager::end(PerformanceEntry* e)
{
	//auto this_id = std::this_thread::get_id();
	e->end();

	PR_ASSERT(mCurrentParent == e, "current parent has to be the calling entry");
	mCurrentParent = e->parent();
}

inline const std::map<std::thread::id, PerformanceManager::Set>& PerformanceManager::entries() const
{
	return mEntries;
}
}
