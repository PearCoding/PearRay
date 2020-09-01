#pragma once

#include "PR_Config.h"
#include <QStyledItemDelegate>

namespace PR {
namespace UI {
class PR_LIB_UI PropertyItemDelegate : public QStyledItemDelegate {
public:
	PropertyItemDelegate(QObject* parent = nullptr);
	virtual ~PropertyItemDelegate();

	QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option,
						  const QModelIndex& index) const override;

	void setEditorData(QWidget* editor, const QModelIndex& index) const override;
	void setModelData(QWidget* editor, QAbstractItemModel* model,
					  const QModelIndex& index) const override;

	void updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option,
							  const QModelIndex& index) const override;
};
} // namespace UI
} // namespace PR