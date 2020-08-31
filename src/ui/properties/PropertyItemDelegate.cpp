#include "PropertyItemDelegate.h"
#include "IProperty.h"

#include <QLineEdit>

namespace PRUI {
PropertyItemDelegate::PropertyItemDelegate(QObject* parent)
	: QStyledItemDelegate(parent)
{
}

PropertyItemDelegate::~PropertyItemDelegate() {}

QWidget* PropertyItemDelegate::createEditor(QWidget* parent, const QStyleOptionViewItem& /*option*/,
											const QModelIndex& index) const
{
	if (!index.isValid())
		return nullptr;

	IProperty* item = static_cast<IProperty*>(index.internalPointer());
	if (!item)
		return nullptr;

	if (index.column() == 0) { // Name
		QLineEdit* lineEdit = new QLineEdit(parent);
		lineEdit->setAutoFillBackground(true);
		lineEdit->setText(item->propertyName());
		return lineEdit;
	} else {
		QWidget* widget = item->editorWidget(parent);
		widget->setAutoFillBackground(true);
		return widget;
	}
}

void PropertyItemDelegate::setEditorData(QWidget* editor, const QModelIndex& index) const
{
	if (!index.isValid())
		return;

	IProperty* item = static_cast<IProperty*>(index.internalPointer());
	if (!item)
		return;

	if (index.column() == 0) { // Name
		QLineEdit* lineEdit = static_cast<QLineEdit*>(editor);
		lineEdit->setText(item->propertyName());
	} else {
		// Already handled inside property
	}
}

void PropertyItemDelegate::setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const
{
	if (!index.isValid())
		return;

	if (index.column() == 0) { // Name
		QLineEdit* lineEdit = static_cast<QLineEdit*>(editor);
		model->setData(index, lineEdit->text(), Qt::EditRole);
	} else {
		// Already handled inside property
	}
}

void PropertyItemDelegate::updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex&) const
{
	editor->setGeometry(option.rect);
}

} // namespace PRUI