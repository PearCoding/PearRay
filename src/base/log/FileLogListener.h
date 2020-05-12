#pragma once

#include "LogListener.h"
#include <fstream>

namespace PR {
class PR_LIB_BASE FileLogListener : public LogListener {
public:
	FileLogListener();
	virtual ~FileLogListener();

	void open(const std::string& file);

	virtual void startEntry(LogLevel level) override;
	virtual void writeEntry(int c) override;

private:
	std::fstream mStream;
};
} // namespace PR
