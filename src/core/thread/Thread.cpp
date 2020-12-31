#include "Thread.h"

namespace PR {

std::atomic<uint32> Thread::sThreadCount(0);
//std::mutex Thread::sGeneralThreadMutex;

void Thread::start()
{
	//mThreadMutex.lock();
	if (mState == State::Waiting && !mThread) {
		mShouldStop = false;
		mState		= State::Running;
		mThread		= new std::thread([](Thread* ptr) {
			ptr->main();
			ptr->mState = State::Stopped;
		},
								  this);
	}
	//mThreadMutex.unlock();
}
} // namespace PR
