#include "Thread.h"

namespace PR {

std::atomic<uint32> Thread::sThreadCount(0);
//std::mutex Thread::sGeneralThreadMutex;

void Thread::start()
{
	//mThreadMutex.lock();
	if (mState == S_Waiting && !mThread) {
		mShouldStop = false;
		mState		= S_Running;
		mThread		= new std::thread([](Thread* ptr) {
			ptr->main();
			ptr->mState = Thread::S_Stopped;
		},
								  this);
	}
	//mThreadMutex.unlock();
}
}
