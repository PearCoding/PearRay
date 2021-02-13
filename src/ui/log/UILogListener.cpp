#include "UILogListener.h"
#include "LogTableModel.h"

#include <QDateTime>

#include <thread>

namespace PR {
namespace UI {
UILogListener::UILogListener(LogTableModel* model)
	: LogListener()
	, mModel(model)
	, mCurrentEntry()
	, mFlushed(true)
{
}

UILogListener::~UILogListener()
{
	if (!mFlushed)
		flush();
}

void UILogListener::flush()
{
	mModel->addEntry(mCurrentEntry);
	mCurrentEntry.Message.clear();
	mFlushed = true;
}

void UILogListener::startEntry(LogLevel level)
{
	if (!mFlushed)
		flush();

	mCurrentEntry.Level		= level;
	mCurrentEntry.ThreadID	= 0; //(quint32)std::this_thread::get_id();
	mCurrentEntry.TimeStamp = QDateTime::currentMSecsSinceEpoch();
	mCurrentEntry.Message.clear();
	mFlushed = false;
}

void UILogListener::writeEntry(int c)
{
	if (c == '\n' && !mFlushed) {
		flush();
	} else {
		mCurrentEntry.Message.append(QChar(c));
		mFlushed = false;
	}
}
} // namespace UI
} // namespace PR
