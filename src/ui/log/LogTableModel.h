#pragma once

#include "PR_Config.h"
#include <QAbstractTableModel>

namespace PR {
namespace UI {
struct LogEntry;
class PR_LIB_UI LogTableModel : public QAbstractTableModel {
	Q_OBJECT

public:
	LogTableModel(QObject* parent = 0);
    virtual ~LogTableModel();

    void addEntry(const LogEntry& entry);
    void addSeparator();
    void reset();

	int rowCount(const QModelIndex& parent = QModelIndex()) const override;
	int columnCount(const QModelIndex& parent = QModelIndex()) const override;

	QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
	QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

private:
	QVector<LogEntry> mEntries;
};
} // namespace UI
} // namespace PR