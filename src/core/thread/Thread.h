#pragma once

#include "PR_Config.h"
#include <atomic>
#include <mutex>
#include <thread>

namespace PR {
/**
	* @brief Represent a parallel running thread
	* @ingroup Core
	* @attention This class should only be accessed by the created (or other managing thread) and the thread itself!
	*/
class PR_LIB_CORE Thread {
public:
	/**
		* @brief The allowed states
		*/
	enum class State {
		Running = 0,
		Waiting = 1,
		Stopped = 2
	};

	/**
		* @brief Create a thread
		*/
	inline Thread();

	/**
		* @brief De-constructor
		*/
	inline virtual ~Thread();

	/**
		* @brief Return the current state
		*
		* @return The current state
		*/
	inline State state() const;

	/**
		* @brief Caller thread wait for this thread to join him
		*/
	inline void join();

	/**
		* @brief Start the thread
		*/
	void start();

	/**
		* @brief Request to stop the running thread, but will not wait for it
		*
		* @warning A thread implementation should use shouldStop() to determine if a stop is needed.<br />There is no stop by the operating system himself!
		* @see shouldStop()
		*/
	inline void requestStop();

	/**
		* @brief Stop the running thread.
		*
		* @warning A thread implementation should use shouldStop() to determine if a stop is needed.<br />There is no stop by the operating system himself!
		* @see shouldStop()
		*/
	inline void stop();

	inline std::thread::id id() const;

	/**
		* @brief Return the current thread count in the framework
		*/
	static inline uint32 threadCount();

	/**
		* @brief Return the current hardware thread count
		*
		* Return the current hardware thread count (e.g. number of CPUs or cores or hyperthreading units),<br />
		* or 0 if this information is not available.
		*/
	static inline uint32 hardwareThreadCount();

	/**
		* @brief Check if a stop is requested
		*
		* @see Stop()
		*/
	inline bool shouldStop() const;

protected:
	/**
		* @brief The thread start point
		*/
	virtual void main() = 0;

private:
	static std::atomic<uint32> sThreadCount;

	std::atomic<State> mState;
	std::thread* mThread;

	std::atomic<bool> mShouldStop;
};
}

#include "Thread.inl"
