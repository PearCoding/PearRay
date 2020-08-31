#include "ProfFile.h"

#include <QFile>

ProfFile::ProfFile()
{
}

ProfFile::~ProfFile()
{
}

static QString readString(QIODevice& device)
{
	quint32 size;
	device.read(reinterpret_cast<char*>(&size), sizeof(size));
	return QString::fromLocal8Bit(device.read(size));
}

bool ProfFile::open(const QString& filename)
{
	QFile file(filename);
	if (!file.open(QIODevice::ReadOnly))
		return false;

	QByteArray header = file.read(10);
	if (header != "PR_PROFILE")
		return false;

	// Make sure everything is cleaned up
	mThreadNames.clear();
	mEntries.clear();

	// (Threads)
	quint64 threads;
	file.read(reinterpret_cast<char*>(&threads), sizeof(threads));
	for (quint64 i = 0; i < threads; ++i) {
		mThreadNames.push_back(readString(file));
	}

	// (Description)
	quint64 entries = 0;
	file.read(reinterpret_cast<char*>(&entries), sizeof(entries));
	for (quint64 i = 0; i < entries; ++i) {
		ProfEntry entry;
		entry.Name	   = readString(file);
		entry.Function = readString(file);
		entry.File	   = readString(file);
		file.read(reinterpret_cast<char*>(&entry.Line), sizeof(entry.Line));
		file.read(reinterpret_cast<char*>(&entry.ThreadID), sizeof(entry.ThreadID));
		entry.Category = readString(file);
		mEntries.push_back(entry);
	}

	// (Signals)[TODO]
	quint64 signalCount = 0;
	file.read(reinterpret_cast<char*>(&signalCount), sizeof(signalCount));
	for (quint64 i = 0; i < signalCount; ++i) {
		readString(file);

		quint32 threadID;
		file.read(reinterpret_cast<char*>(&threadID), sizeof(threadID));
		Q_UNUSED(threadID);

		quint64 timepoint;
		file.read(reinterpret_cast<char*>(&timepoint), sizeof(timepoint));
		Q_UNUSED(timepoint);
	}

	// (Pages)
	quint64 pages = 0;
	file.read(reinterpret_cast<char*>(&pages), sizeof(pages));
	for (quint64 i = 0; i < pages; ++i) {
		quint64 timepoint;
		file.read(reinterpret_cast<char*>(&timepoint), sizeof(timepoint));

		quint64 counters;
		file.read(reinterpret_cast<char*>(&counters), sizeof(counters));
		for (quint64 j = 0; j < counters; ++j) {
			quint64 descID;
			file.read(reinterpret_cast<char*>(&descID), sizeof(descID));

			ProfCounterEntry entry;
			file.read(reinterpret_cast<char*>(&entry.Value), sizeof(entry.Value));
			entry.TimePointNS = timepoint;

			mEntries[descID].CounterEntries.push_back(entry);
		}

		quint64 timecounters;
		file.read(reinterpret_cast<char*>(&timecounters), sizeof(timecounters));
		for (quint64 j = 0; j < timecounters; ++j) {
			quint64 descID;
			file.read(reinterpret_cast<char*>(&descID), sizeof(descID));

			ProfTimeCounterEntry entry;
			file.read(reinterpret_cast<char*>(&entry.TotalValue), sizeof(entry.TotalValue));
			file.read(reinterpret_cast<char*>(&entry.TotalDurationNS), sizeof(entry.TotalDurationNS));
			entry.TimePointNS = timepoint;

			mEntries[descID].TimeCounterEntries.push_back(entry);
		}
	}

	// Sort entries
	for (auto& entry : mEntries) {
		std::sort(entry.CounterEntries.begin(), entry.CounterEntries.end(),
				  [](const ProfCounterEntry& a, const ProfCounterEntry& b) {
					  return a.TimePointNS < b.TimePointNS;
				  });
		std::sort(entry.TimeCounterEntries.begin(), entry.TimeCounterEntries.end(),
				  [](const ProfTimeCounterEntry& a, const ProfTimeCounterEntry& b) {
					  return a.TimePointNS < b.TimePointNS;
				  });
	}

	// Sort descriptions, such that same file decs are next to eachother
	std::sort(mEntries.begin(), mEntries.end(),
			  [](const ProfEntry& a, const ProfEntry& b) {
				  return a.File < b.File;
			  });

	return true;
}