#pragma once

#include "Logger.h"
#include <QString>

namespace PR {
namespace UI {
struct PR_LIB_UI LogEntry {
	LogLevel Level;
	qint64 TimeStamp;
	quint32 ThreadID;
	QString Message;
};
} // namespace UI
} // namespace PR