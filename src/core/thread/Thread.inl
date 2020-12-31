// IWYU pragma: private, include "thread/Thread.h"
namespace PR {
inline Thread::Thread()
	: mState(State::Waiting)
	, mThread(nullptr)
	, mShouldStop(false)
{
	sThreadCount++;
}

inline Thread::~Thread()
{
	sThreadCount--;

	if (mThread) {
		if (mThread->joinable())
			mThread->join();
		delete mThread;
	}
}

inline Thread::State Thread::state() const
{
	return mState;
}

inline void Thread::join()
{
	if (mState != State::Running || !mThread || mThread->get_id() == std::this_thread::get_id() || !mThread->joinable())
		return;

	mThread->join();
}

inline void Thread::requestStop()
{
	if (mState != State::Running || !mThread)
		return;

	mShouldStop = true;
}

inline void Thread::stop()
{
	if (mState != State::Running || !mThread)
		return;
	requestStop();

	mThread->join();

	delete mThread;
	mThread = nullptr;
}

inline std::thread::id Thread::id() const
{
	return mThread->get_id();
}

inline uint32 Thread::threadCount()
{
	return sThreadCount;
}

inline uint32 Thread::hardwareThreadCount()
{
	return std::thread::hardware_concurrency();
}

inline bool Thread::shouldStop() const
{
	return mShouldStop;
}
} // namespace PR
