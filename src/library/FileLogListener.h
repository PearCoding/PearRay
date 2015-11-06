#pragma once

#include "LogListener.h"
#include <fstream>

namespace PR
{
	class PR_LIB FileLogListener : public LogListener
	{
	public:
		FileLogListener();
		virtual ~FileLogListener();

		void open(const std::string& file);

		virtual void newEntry(Level level, Module m, const std::string& str);

	private:
		std::fstream mStream;
	};
}
