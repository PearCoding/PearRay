#pragma once

#include <QTableView>

#include "PR_Config.h"

namespace PR {
namespace UI {
/// Simple log view with automatic scroll to the bottom feature
class PR_LIB_UI LogView : public QTableView {
	Q_OBJECT
public:
	explicit LogView(QWidget* parent = nullptr);
	virtual ~LogView();

	virtual void setModel(QAbstractItemModel* model) override;

private slots:
	virtual void entriesAboutToBeInserted(const QModelIndex& parent, int start, int end);
	virtual void entriesInserted(const QModelIndex& parent, int start, int end);

private:
	bool mAtEnd;
};
} // namespace UI
} // namespace PR