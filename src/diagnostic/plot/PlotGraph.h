#pragma once

#include <QObject>

class IValueProvider;
class PlotGraph : public QObject {
	Q_OBJECT
public:
	PlotGraph(IValueProvider* provider, QObject* parent = nullptr);
	virtual ~PlotGraph();

	float xAt(int index) const;
	float yAt(int index) const;
	int elementCount() const;

signals:
	void dataChanged();

private slots:
	void updateData();

private:
	IValueProvider* mProvider;
};