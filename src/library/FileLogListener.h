#pragma once

#include "LogListener.h"
#include <fstream>

namespace PR {
class PR_LIB FileLogListener final : public LogListener {
public:
	FileLogListener();
	virtual ~FileLogListener();

	void open(const std::string& file);

	void startEntry(LogLevel level) override;
	void writeEntry(int c) override;

private:
	std::fstream mStream;
};
}
