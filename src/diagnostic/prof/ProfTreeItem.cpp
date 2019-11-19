#include "ProfTreeItem.h"
#include "io/ProfFile.h"

#include <QDebug>

ProfTreeItem::ProfTreeItem(const std::shared_ptr<ProfTreeItem>& parent, QString name, ProfFile* file, int index)
	: std::enable_shared_from_this<ProfTreeItem>()
	, mName(name)
	, mParent(parent)
	, mFile(file)
	, mIndex(index)
{
	if (isLeaf())
		Q_ASSERT(!mFile->entry(mIndex).TimeCounterEntries.isEmpty());
}

ProfTreeItem::~ProfTreeItem()
{
}

static int indexOfEntry(const QVector<ProfTimeCounterEntry>& entries,
						quint64 t)
{
	if (entries.isEmpty() || entries.front().TimePointMicroSec > t)
		return -1;

	int L = 0;
	int R = entries.size();

	while (L < R) {
		int h		 = (L + R) / 2;
		quint64 time = entries.at(h).TimePointMicroSec;
		if (time < t)
			L = h + 1;
		else
			R = h;
	}

	return std::min(entries.size() - 1, L);
}

quint64 ProfTreeItem::totalValue() const
{
	if (isLeaf()) {
		return mFile->entry(mIndex).TimeCounterEntries.last().TotalValue;
	} else {
		quint64 value = 0;
		for (std::shared_ptr<ProfTreeItem> child : mChildren) {
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
		for (std::shared_ptr<ProfTreeItem> child : mChildren) {
			value = child->totalDuration();
		}
		return value;
	}
}

quint64 ProfTreeItem::totalValue(quint64 t) const
{
	if (isLeaf()) {
		int index = indexOfEntry(mFile->entry(mIndex).TimeCounterEntries, t);
		if (index < 0)
			return 0;
		else
			return mFile->entry(mIndex).TimeCounterEntries.at(index).TotalDuration;
	} else {
		quint64 value = 0;
		for (std::shared_ptr<ProfTreeItem> child : mChildren) {
			value = child->totalValue(t);
		}
		return value;
	}
}

quint64 ProfTreeItem::totalDuration(quint64 t) const
{
	if (isLeaf()) {
		int index = indexOfEntry(mFile->entry(mIndex).TimeCounterEntries, t);
		if (index < 0)
			return 0;
		else
			return mFile->entry(mIndex).TimeCounterEntries.at(index).TotalValue;
	} else {
		quint64 value = 0;
		for (std::shared_ptr<ProfTreeItem> child : mChildren) {
			value = child->totalDuration(t);
		}
		return value;
	}
}

template <typename T>
static void mergeSortedArrays(QVector<T>& a, const QVector<T>& b)
{
	if (a.isEmpty()) {
		a = b;
		return;
	}

	int ai = 0;
	for (int bi = 0; bi < b.size();) {
		if (ai < a.size()) {
			if (a.at(ai) == b.at(bi)) { // Ignore
				++ai;
				++bi;
			} else if (a.at(ai) < b.at(bi)) {
				++ai;
				// Fix bi
			} else {
				a.insert(ai, b.at(bi));
				++ai;
				++bi;
			}
		} else {
			a.append(b.at(bi));
			++ai;
			++bi;
		}
	}
}

QVector<quint64> ProfTreeItem::timePoints() const
{
	QVector<quint64> points;
	if (isLeaf()) {
		points.reserve(mFile->entry(mIndex).TimeCounterEntries.size());
		for (const ProfTimeCounterEntry& entry : mFile->entry(mIndex).TimeCounterEntries)
			points.append(entry.TimePointMicroSec);
	} else {
		for (std::shared_ptr<ProfTreeItem> child : mChildren)
			mergeSortedArrays(points, child->timePoints());
	}

	//qDebug() << points;
	return points;
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
