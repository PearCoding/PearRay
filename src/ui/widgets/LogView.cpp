#include "LogView.h"

#include <QScrollBar>

namespace PR {
namespace UI {
LogView::LogView(QWidget* parent)
	: QTableView(parent)
	, mAtEnd(true)
{
}

LogView::~LogView()
{
}

void LogView::setModel(QAbstractItemModel* model)
{
	QAbstractItemModel* previousModel = this->model();
	if (previousModel)
		disconnect(previousModel);

	connect(model, &QAbstractItemModel::rowsAboutToBeInserted, this, &LogView::entriesAboutToBeInserted);
	connect(model, &QAbstractItemModel::rowsInserted, this, &LogView::entriesInserted);

	QTableView::setModel(model);
}

void LogView::entriesAboutToBeInserted(const QModelIndex&, int, int)
{
	auto scrollbar = verticalScrollBar();
	mAtEnd		   = scrollbar->value() == scrollbar->maximum();
}

void LogView::entriesInserted(const QModelIndex&, int, int)
{
	if (mAtEnd)
		scrollToBottom();
}
} // namespace UI
} // namespace PR