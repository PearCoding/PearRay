#pragma once

#include "PR_Config.h"
#include <atomic>
#include <mutex>
#include <thread>

namespace PR
{
	/**
	* @brief Represent a parallel running thread
	* @ingroup Core
	* @attention This class should only be accessed by the created (or other managing thread) and the thread itself!
	*/
	class PR_LIB Thread
	{
		friend void* threadWrapper(void*);
	public:
		/**
		* @brief The allowed states
		*/
		enum _State
		{
			S_Running = 0,
			S_Waiting = 1,
			S_Stopped = 2
		};

		/**
		* @brief Create a thread
		*/
		inline explicit Thread();

		/**
		* @brief De-constructor
		*/
		inline virtual ~Thread();

		/**
		* @brief Return the current state
		*
		* @return The current state
		*/
		inline _State state() const;

		/**
		* @brief Caller thread wait for this thread to join him
		*/
		inline void join();

		/**
		* @brief Start the thread
		*/
		void start();

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
		//static std::mutex sGeneralThreadMutex;

		_State mState;
		std::thread* mThread;

		bool mShouldStop;
		//mutable std::mutex mThreadMutex;
	};
}

#include "Thread.inl"
