#pragma once

#include <QVector>

struct ProfCounterEntry {
	quint64 Value;
	quint64 TimePointMicroSec;
};

struct ProfTimeCounterEntry {
	quint64 TotalValue;
	quint64 TotalDuration;
	quint64 TimePointMicroSec;
};

struct ProfEntry {
	QString Name;
	QString Function;
	QString File;
	quint32 Line;
	quint32 ThreadID;
	QString Category;

	QVector<ProfCounterEntry> CounterEntries;
	QVector<ProfTimeCounterEntry> TimeCounterEntries;
};

class ProfFile {
public:
	ProfFile();
	virtual ~ProfFile();

	bool open(const QString& file);

	inline size_t entryCount() const { return mEntries.size(); }
	inline const ProfEntry& entry(int i) const { return mEntries.at(i); }
	inline const QVector<ProfEntry>& entries() const { return mEntries; }

	inline size_t threadCount() const { return mThreadNames.size(); }
	inline const QString& threadName(int i) const { return mThreadNames.at(i); }
	inline const QVector<QString>& threadNames() const { return mThreadNames; }

private:
	QVector<ProfEntry> mEntries;
	QVector<QString> mThreadNames;
};