#pragma once

#include "Config.h"
#include <list>
#include <string>

namespace PR
{
	enum Level
	{
		L_Debug = 0,
		L_Info,
		L_Warning,
		L_Error,
		L_Fatal
	};

	enum Module
	{
		M_Internal = 0,
		M_Test,
		M_System,
		M_Scene,
		M_Volume,
		M_Ray,
		M_Entity,
		M_World,
	};

	class LogListener;
	class PR_LIB Logger
	{
	public:

		Logger();
		~Logger();

		static const char* levelString(Level l);
		static const char* moduleString(Module m);

		void log(Level level, Module m, const std::string& str);
		void logf(Level level, Module m, const char* fmt, ...);
		void addListener(LogListener* listener);
		void removeListener(LogListener* listener);

		inline void setVerbose(bool b)
		{
			mVerbose = b;
		}

		inline bool isVerbose() const
		{
			return mVerbose;
		}

		static inline Logger& instance()
		{
			static Logger log;
			return log;
		}

	private:
		std::list<LogListener*> mListener;
		bool mVerbose;
	};
}

#define PR_LOGGER (PR::Logger::instance())