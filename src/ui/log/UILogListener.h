#pragma once

#include "log/LogListener.h"
#include "LogEntry.h"

namespace PR {
namespace UI {
class LogTableModel;
class PR_LIB_UI UILogListener : public LogListener {
public:
	UILogListener(LogTableModel* model);
	virtual ~UILogListener();

	virtual void startEntry(LogLevel level) override;
	virtual void writeEntry(int c) override;

private:
	void flush();

	LogTableModel* mModel;

	LogEntry mCurrentEntry;
	bool mFlushed;
};
} // namespace UI
} // namespace PR
