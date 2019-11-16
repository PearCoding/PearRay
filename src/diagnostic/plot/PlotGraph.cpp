#include "PlotGraph.h"
#include "IValueProvider.h"

PlotGraph::PlotGraph(IValueProvider* provider, QObject* parent)
	: QObject(parent)
	, mProvider(provider)
{
	connect(mProvider, SIGNAL(dataChanged()), this, SLOT(updateData()));
}

PlotGraph::~PlotGraph()
{
}

float PlotGraph::xAt(int index) const
{
	return mProvider->xAt(index);
}

float PlotGraph::yAt(int index) const
{
	return mProvider->yAt(index);
}

int PlotGraph::elementCount() const
{
	return mProvider->elementCount();
}

void PlotGraph::updateData()
{

	emit dataChanged();
}