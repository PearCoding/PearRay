#pragma once

#include "LogListener.h"
#include <fstream>

namespace PR {
class PR_LIB FileLogListener final : public LogListener {
public:
	FileLogListener();
	virtual ~FileLogListener();

	void open(const std::string& file);
	void newEntry(Level level, Module m, const std::string& str) override;

private:
	std::fstream mStream;
};
}
