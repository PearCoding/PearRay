#pragma once

namespace PR {
inline Thread::Thread()
	: mState(S_Waiting)
	, mThread(nullptr)
	, mShouldStop(false) //, mThreadMutex()
{
	//sGeneralThreadMutex.lock();
	sThreadCount++;
	//sGeneralThreadMutex.unlock();
}

inline Thread::~Thread()
{
	//sGeneralThreadMutex.lock();
	sThreadCount--;
	//sGeneralThreadMutex.unlock();

	if (mThread) {
		mThread->detach();
		delete mThread;
	}
}

inline Thread::_State Thread::state() const
{
	//std::lock_guard<std::mutex> guard(mThreadMutex);
	return mState;
}

inline void Thread::join()
{
	//mThreadMutex.lock();
	if (mState != S_Running || !mThread || mThread->get_id() == std::this_thread::get_id() || !mThread->joinable()) {
		//mThreadMutex.unlock();
		return;
	}
	//mThreadMutex.unlock();

	mThread->join();
}

inline void Thread::stop()
{
	//mThreadMutex.lock();
	if (mState != S_Running || !mThread) {
		//mThreadMutex.unlock();
		return;
	}

	mShouldStop = true;
	//mThreadMutex.unlock();

	mThread->join();

	//mThreadMutex.lock();
	delete mThread;
	mThread = nullptr;
	//mThreadMutex.unlock();
}

inline std::thread::id Thread::id() const
{
	//std::lock_guard<std::mutex> guard(mThreadMutex);
	return mThread->get_id();
}

inline uint32 Thread::threadCount()
{
	//std::lock_guard<std::mutex> guard(sGeneralThreadMutex);
	return sThreadCount;
}

inline uint32 Thread::hardwareThreadCount()
{
	return std::thread::hardware_concurrency();
}

inline bool Thread::shouldStop() const
{
	//std::lock_guard<std::mutex> guard(mThreadMutex);
	return mShouldStop;
}
}
