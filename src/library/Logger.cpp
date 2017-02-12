#include "Logger.h"
#include "LogListener.h"

#include <stdio.h>
#include <stdarg.h>
#include <string>

namespace PR
{
	Logger::Logger() :
		mVerbose(false), mQuiet(false)
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
		"Internal  ",
		"Test      ",
		"Camera    ",
		"GPU       ",
		"Entity    ",
		"Math      ",
		"Integrator",
		"Material  ",
		"System    ",
		"Scene     ",
		"Volume    ",
		"Network   ",
		"Loader    ",
		"Shader    ",
		"Main      "
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
			return;

		if(!mQuiet)
			printf("[%s] (%s) %s\n", levelStr[level], moduleStr[m], str.c_str());

		for (std::list<LogListener*>::iterator it = mListener.begin();
			it != mListener.end();
			++it)
		{
			(*it)->newEntry(level, m, str);
		}

		if (level == L_Fatal)
			exit(-1);
	}

	void Logger::logf(Level level, Module m, const char* fmt, ...)
	{
		if (!mVerbose && (level == L_Debug || level == L_Info))
			return;

		// Unsecured
		char buffer[1024];
		va_list vl;
		va_start(vl, fmt);
		vsnprintf(buffer, sizeof(buffer), fmt, vl);
		va_end(vl);

		if(!mQuiet)
			printf("[%s] (%s) %s\n", levelStr[level], moduleStr[m], buffer);

		for (std::list<LogListener*>::iterator it = mListener.begin();
			it != mListener.end();
			++it)
		{
			(*it)->newEntry(level, m, buffer);
		}

		if (level == L_Fatal)
			exit(-1);
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
