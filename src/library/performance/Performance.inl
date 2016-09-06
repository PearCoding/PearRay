#pragma once

namespace PR
{
	inline PerformanceEntry::PerformanceEntry(uint64 hash, PerformanceEntry* parent,
			int line, const char* file, const char* function) :
		mHash(hash), mParent(parent), mLine(line), mFile(file), mFunction(function),
		mStart(), mTicks(0)
	{}
		
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
		for(auto p : mEntries)
			for(auto e : p.second)
				delete e;
	}

	inline PerformanceEntry* PerformanceManager::add(int line, const char* file, const char* function)
	{
		auto this_id = std::this_thread::get_id();

		mMutex.lock();
		PerformanceEntry* parent = nullptr; 
		if(!mEntries.count(this_id))
			mEntries[this_id] = std::list<PerformanceEntry*>();
		else
			parent = mParents[this_id];

		std::stringstream stream;
		stream << function << "$" << file << "$" << line << "$" << this_id;
		if(parent)
			stream << "$" << parent->hash();
		
		size_t hash = std::hash<std::string>()(stream.str());

		PerformanceEntry* entry = nullptr;
		for(auto e : mEntries[this_id])
		{
			if(e->hash() == hash)
			{
				entry = e;
				break;
			}
		}

		if(!entry)
		{
			entry = new PerformanceEntry(hash, parent, line, file, function);
			PR_ASSERT(entry);

			mEntries[this_id].push_back(entry);
		}

		mParents[this_id] = entry;
		mMutex.unlock();

		if(parent)
		{
			bool has = false;
			for(auto e : parent->children())
			{
				if(e->hash() == hash)
				{
					has = true;
					break;
				}
			}

			if(!has)
				parent->children().push_back(entry);
		}

		entry->start();
		return entry;
	}

	inline void PerformanceManager::end(PerformanceEntry* e)
	{
		auto this_id = std::this_thread::get_id();
		e->end();

		mMutex.lock();
		PR_ASSERT(mParents.count(this_id));
		PR_ASSERT(mParents[this_id] == e);
		mParents[this_id] = e->parent();
		mMutex.unlock();
	}

	inline const std::map<std::thread::id, std::list<PerformanceEntry*> >& PerformanceManager::entries() const
	{
		return mEntries;
	}
}
