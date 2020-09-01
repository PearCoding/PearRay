#include "PropertyView.h"
#include "properties/PropertyContainer.h"
#include "properties/PropertyItemDelegate.h"
#include "properties/PropertyTreeModel.h"

#include <QApplication>
#include <QDebug>
#include <QHeaderView>
#include <QPainter>

namespace PR {
namespace UI {
PropertyView::PropertyView(QWidget* parent)
	: QTreeView(parent)
	, mModel(nullptr)
	, mDelegate(new PropertyItemDelegate(this))
{
	setAlternatingRowColors(true);
	setRootIsDecorated(true);
	setSelectionMode(QAbstractItemView::NoSelection);
	setFocusPolicy(Qt::NoFocus);
	setItemDelegate(mDelegate);

	QPalette p(palette());
	p.setColor(QPalette::AlternateBase, QColor(255, 255, 200));
	p.setColor(QPalette::Base, QColor(255, 255, 160));
	setPalette(p);

	connect(header(), SIGNAL(sectionDoubleClicked(int)), this, SLOT(resizeColumnToContents(int)));
	header()->setStretchLastSection(true);
}

PropertyView::~PropertyView()
{
}

void PropertyView::setPropertyContainer(PropertyContainer* table)
{
	PropertyTreeModel* oldmodel = mModel;

	mModel = new PropertyTreeModel(table);
	setModel(mModel);

	if (oldmodel)
		delete oldmodel;
}

PropertyContainer* PropertyView::propertyContainer() const
{
	return mModel->container();
}

void PropertyView::addProperty(IProperty* property)
{
	mModel->addProperty(property);
}

void PropertyView::drawRow(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
	if (!mModel || !index.isValid()) {
		QTreeView::drawRow(painter, option, index);
		return;
	}

	IProperty* property = static_cast<IProperty*>(index.internalPointer());
	if (property->isHeader()) { //Group
		painter->save();
		painter->setBrush(QColor(180, 180, 180));
		painter->setPen(Qt::NoPen);
		painter->drawRect(option.rect);
		painter->restore();

		// Really bad hack to prevent the alternating color on top of the header color!
		const_cast<PropertyView*>(this)->setAlternatingRowColors(false);
	}

	QTreeView::drawRow(painter, option, index);

	if (property->isHeader())
		const_cast<PropertyView*>(this)->setAlternatingRowColors(true);

	painter->save();
	painter->setPen(QPen(Qt::gray));
	painter->drawLine(option.rect.x(), option.rect.bottom(), option.rect.right(), option.rect.bottom());
	painter->restore();
}

void PropertyView::drawBranches(QPainter* painter, const QRect& rect, const QModelIndex& index) const
{
	QTreeView::drawBranches(painter, rect, index);

	if (!mModel || !index.isValid())
		return;

	IProperty* property = static_cast<IProperty*>(index.internalPointer());
	if (!property->isHeader()) {
		painter->save();
		painter->setPen(QPen(Qt::gray));
		painter->drawLine(rect.right(), rect.top(), rect.right(), rect.bottom());
		painter->restore();
	}
}
} // namespace UI
} // namespace PR