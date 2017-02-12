#include "Thread.h"

namespace PR
{
	/**
	* @brief A wrapper to call Thread::main()!
	*
	* @internal
	* @see Thread::main()
	* @warning Only a wrapper!
	*/
	void* threadWrapper(void* ptr)
	{
		((Thread*)ptr)->main();

		//((Thread*)ptr)->mThreadMutex.lock();
		((Thread*)ptr)->mState = Thread::S_Stopped;
		//((Thread*)ptr)->mThreadMutex.unlock();

		return 0;
	}

	std::atomic<uint32> Thread::sThreadCount(0);
	//std::mutex Thread::sGeneralThreadMutex;

	void Thread::start()
	{
		//mThreadMutex.lock();
		if (mState == S_Waiting && !mThread)
		{
			mShouldStop = false;
			mThread = new std::thread(threadWrapper, this);
			mState = S_Running;
		}
		//mThreadMutex.unlock();
	}
}
