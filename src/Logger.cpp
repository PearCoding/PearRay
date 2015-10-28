#include "Logger.h"
#include "LogListener.h"

#include <stdio.h>
#include <stdarg.h>
#include <string>

namespace PR
{
	Logger::Logger() :
#ifdef PR_DEBUG
		mVerbose(true)
#else
		mVerbose(false)
#endif
	{
	}

	Logger::~Logger()
	{
	}

	const char* levelStr[] =
	{
		"Debug  ",
		"Info   ",
		"Warning",
		"Error  ",
		"Fatal  "
	};

	const char* moduleStr[] =
	{
		"Internal",
		"Test",
		"System  ",
		"Scene ",
		"Volume ",
		"Ray  ",
		"Entity  ",
		"World   ",
	};

	const char* Logger::levelString(Level l)
	{
		return levelStr[l];
	}

	const char* Logger::moduleString(Module m)
	{
		return moduleStr[m];
	}

	void Logger::log(Level level, Module m, const std::string& str)
	{
		if (!mVerbose && (level == L_Debug || level == L_Info))
		{
			return;
		}
				
		printf("[%s] (%s) %s", levelStr[level], moduleStr[m], str.c_str());

		for (std::list<LogListener*>::iterator it = mListener.begin();
			it != mListener.end();
			++it)
		{
			(*it)->newEntry(level, m, str);
		}

		printf("\n");

		if (level == L_Fatal)
		{
			exit(-1);
		}
	}

	void Logger::logf(Level level, Module m, const char* fmt, ...)
	{
		if (!mVerbose && (level == L_Debug || level == L_Info))
		{
			return;
		}

		//Unsecure
		char buffer[1024];
		va_list vl;
		va_start(vl, fmt);
		vsnprintf(buffer, 1024, fmt, vl);
		va_end(vl);

		printf("[%s] (%s) %s", levelStr[level], moduleStr[m], buffer);

		for (std::list<LogListener*>::iterator it = mListener.begin();
			it != mListener.end();
			++it)
		{
			(*it)->newEntry(level, m, buffer);
		}

		printf("\n");

		if (level == L_Fatal)
		{
			exit(-1);
		}
	}

	void Logger::addListener(LogListener* listener)
	{
		mListener.push_back(listener);
	}

	void Logger::removeListener(LogListener* listener)
	{
		mListener.remove(listener);
	}
}