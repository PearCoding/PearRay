#include "ProfTreeItem.h"
#include "io/ProfFile.h"

#include <QDateTime>

ProfTreeItem::ProfTreeItem(const std::shared_ptr<ProfTreeItem>& parent, QString name, ProfFile* file, int index)
	: std::enable_shared_from_this<ProfTreeItem>()
	, mName(name)
	, mParent(parent)
	, mFile(file)
	, mIndex(index)
{
	if (isLeaf()) {
		Q_ASSERT(!mFile->entry(mIndex).TimeCounterEntries.isEmpty());
	}
}

ProfTreeItem::~ProfTreeItem()
{
}

quint64 ProfTreeItem::totalValue() const
{
	if (isLeaf()) {
		return mFile->entry(mIndex).TimeCounterEntries.last().TotalValue;
	} else {
		quint64 value = 0;
		for (const auto& child : mChildren) {
			value = child->totalValue();
		}
		return value;
	}
}

quint64 ProfTreeItem::totalDuration() const
{
	if (isLeaf()) {
		return mFile->entry(mIndex).TimeCounterEntries.last().TotalDuration;
	} else {
		quint64 value = 0;
		for (const auto& child : mChildren) {
			value = child->totalDuration();
		}
		return value;
	}
}

std::shared_ptr<ProfTreeItem> ProfTreeItem::child(int row) const
{
	if (row < 0 || row >= mChildren.size())
		return nullptr;

	return mChildren.at(row);
}

QString durationToString(quint64 dur)
{
	quint64 micsecs = dur % 1000;
	dur /= 1000;
	quint64 milsecs = dur % 1000;
	dur /= 1000;
	quint64 secs = dur % 60;
	dur /= 60;
	quint64 mins = dur % 60;
	dur /= 60;
	quint64 hours = dur;

	if (hours > 0)
		return QString("%1:%2:%3.%4.%5").arg(hours, 2, 10, QChar('0')).arg(mins, 2, 10, QChar('0')).arg(secs, 2, 10, QChar('0')).arg(milsecs, 3, 10, QChar('0')).arg(micsecs, 3, 10, QChar('0'));
	else if (mins > 0)
		return QString("%1:%2.%3.%4").arg(mins, 2, 10, QChar('0')).arg(secs, 2, 10, QChar('0')).arg(milsecs, 3, 10, QChar('0')).arg(micsecs, 3, 10, QChar('0'));
	else
		return QString("%1.%2.%3").arg(secs, 2, 10, QChar('0')).arg(milsecs, 3, 10, QChar('0')).arg(micsecs, 3, 10, QChar('0'));
}

QVariant ProfTreeItem::data(int column) const
{
	switch (column) {
	case C_Name:
		return mName;
	case C_TotalValue:
		return totalValue();
	case C_TotalDuration:
		return durationToString(totalDuration());
	case C_AverageDuration:
		return durationToString(totalDuration() / totalValue());
	default:
		return QVariant();
	}
}

int ProfTreeItem::row() const
{
	if (mParent)
		return mParent->mChildren.indexOf(std::const_pointer_cast<ProfTreeItem>(shared_from_this()));

	return 0;
}

void ProfTreeItem::addChild(const std::shared_ptr<ProfTreeItem>& item)
{
	Q_ASSERT(item);
	Q_ASSERT(!isLeaf());
	mChildren.push_back(item);
}

void ProfTreeItem::removeChild(const std::shared_ptr<ProfTreeItem>& item)
{
	Q_ASSERT(item);
	Q_ASSERT(!isLeaf());
	mChildren.removeOne(item);
}
