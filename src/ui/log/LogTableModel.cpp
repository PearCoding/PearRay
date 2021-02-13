#include "LogTableModel.h"
#include "LogEntry.h"

#include <QBrush>
#include <QDateTime>

namespace PR {
namespace UI {
constexpr int ColumnCount = 4;

LogTableModel::LogTableModel(QObject* parent)
	: QAbstractTableModel(parent)
{
}

LogTableModel::~LogTableModel() {}

void LogTableModel::addEntry(const LogEntry& entry)
{
	int row = rowCount(QModelIndex());
	beginInsertRows(QModelIndex(), row, row);
	mEntries.append(entry);
	endInsertRows();
}

void LogTableModel::addSeparator()
{
}

void LogTableModel::reset()
{
	beginResetModel();
	mEntries.clear();
	endResetModel();
}

int LogTableModel::rowCount(const QModelIndex& parent) const
{
	if (parent.isValid())
		return 0;
	return mEntries.size();
}

int LogTableModel::columnCount(const QModelIndex& parent) const
{
	if (parent.isValid())
		return 0;
	return ColumnCount;
}

QVariant LogTableModel::data(const QModelIndex& index, int role) const
{
	if (!index.isValid())
		return QVariant();

	if (index.row() >= mEntries.size() || index.row() < 0)
		return QVariant();

	const auto& entry = mEntries[index.row()];

	if (role == Qt::ToolTipRole) {
		switch (index.column()) {
		default:
			return QVariant();
		case 0:
			return QDateTime::fromMSecsSinceEpoch(entry.TimeStamp).toString(Qt::ISODateWithMs);
		case 1:
			return Logger::levelString(entry.Level);
		case 2:
			return entry.ThreadID;
		case 3:
			return entry.Message;
		}
	} else if (role == Qt::DisplayRole) {
		switch (index.column()) {
		default:
			return QVariant();
		case 0:
			return QDateTime::fromMSecsSinceEpoch(entry.TimeStamp).toString("hh:mm:ss.zzz");
		case 1:
			return Logger::levelString(entry.Level);
		case 2:
			return entry.ThreadID;
		case 3:
			return entry.Message;
		}
	} else if (role == Qt::BackgroundRole) {
		switch (entry.Level) {
		default:
		case L_INFO:
			return QBrush();
		case L_DEBUG:
			return QBrush(Qt::lightGray);
		case L_WARNING:
			return QBrush(qRgb(255, 228, 153));
		case L_ERROR:
			return QBrush(qRgb(255, 177, 153));
		case L_FATAL:
			return QBrush(qRgb(252, 109, 109));
		}
	} else {
		return QVariant();
	}
}

QVariant LogTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	if (role != Qt::DisplayRole || orientation == Qt::Vertical)
		return QVariant();

	switch (section) {
	default:
		return QVariant();
	case 0:
		return tr("Time");
	case 1:
		return tr("Verbosity");
	case 2:
		return tr("Thread");
	case 3:
		return tr("Message");
	}
}
} // namespace UI
} // namespace PR