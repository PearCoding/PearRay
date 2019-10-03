#include "ColorButton.h"

#include <QApplication>
#include <QtEvents>
#include <QColorDialog>
#include <QPainter>
#include <QMimeData>
#include <QStyle>
#include <QStyleOption>
#include <QDrag>

#define PS_COLOR_BUTTON_PADDING (4)

ColorButton::ColorButton(QWidget *parent) :
	QPushButton(parent), mColor(Qt::black), mMousePressed(false),
	mIsFlat(false)
{
	setAcceptDrops(true);
	connect(this, SIGNAL(clicked()), SLOT(changeColor()));
}

ColorButton::~ColorButton()
{
}

void ColorButton::setFlat(bool b)
{
	mIsFlat = b;
	repaint();
}

bool ColorButton::isFlat() const
{
	return mIsFlat;
}

const QColor& ColorButton::color() const
{
	return mColor;
}

void ColorButton::setColor(const QColor &c)
{
	mColor = c;
	update();
}

void ColorButton::changeColor()
{
	QColor c = QColorDialog::getColor(mColor, qApp->activeWindow());
	if (c.isValid())
	{
		setColor(c);
		emit colorChanged(color());
	}
}

QSize ColorButton::sizeHint() const
{
	return QSize(80, 25);
}

QSize ColorButton::minimumSizeHint() const
{
	return QSize(80, 25);
}

void ColorButton::drawButton(QPainter *p)
{
	QStyleOptionButton buttonOptions;
	buttonOptions.init(this);
	buttonOptions.features = QStyleOptionButton::None;
	buttonOptions.rect = rect();
	buttonOptions.palette = palette();
	buttonOptions.state = (isDown() ? QStyle::State_Sunken : QStyle::State_Raised);

	if (!mIsFlat)
	{
		style()->drawPrimitive(QStyle::PE_PanelButtonBevel, &buttonOptions, p, this);
	}
	else
	{
		style()->drawPrimitive(QStyle::PE_FrameDefaultButton, &buttonOptions, p, this);
	}

	p->save();
	drawButtonLabel(p);
	p->restore();

	QStyleOptionFocusRect frectOptions;
	frectOptions.init(this);
	frectOptions.rect = style()->subElementRect(QStyle::SE_PushButtonFocusRect, &buttonOptions, this);
	if (hasFocus())
	{
		style()->drawPrimitive(QStyle::PE_FrameFocusRect, &frectOptions, p, this);
	}
}

void ColorButton::drawButtonLabel(QPainter *p)
{
	QPalette::ColorGroup cg =
		(isEnabled() ? (hasFocus() ? QPalette::Active : QPalette::Inactive) : QPalette::Disabled);
	p->setPen(palette().color(cg, QPalette::ButtonText));
	p->setBrush(mColor);
	p->drawRect(PS_COLOR_BUTTON_PADDING, PS_COLOR_BUTTON_PADDING,
		width() - PS_COLOR_BUTTON_PADDING * 2 - 1, height() - PS_COLOR_BUTTON_PADDING * 2 - 1);
}

void ColorButton::dragEnterEvent(QDragEnterEvent *e)
{
	if (!e->mimeData()->hasColor())
	{
		e->ignore();
		return;
	}
}

void ColorButton::dragMoveEvent(QDragMoveEvent *e)
{
	if (!e->mimeData()->hasColor())
	{
		e->ignore();
		return;
	}
	e->accept();
}

void ColorButton::dropEvent(QDropEvent *e)
{
	if (!e->mimeData()->hasColor())
	{
		e->ignore();
		return;
	}

	QColor c = qvariant_cast<QColor>(e->mimeData()->colorData());
	setColor(c);
	emit colorChanged(color());
}

void ColorButton::mousePressEvent(QMouseEvent *e)
{
	mPressPos = e->pos();
	mMousePressed = true;
	QPushButton::mousePressEvent(e);
}

void ColorButton::mouseReleaseEvent(QMouseEvent *e)
{
	mMousePressed = false;
	QPushButton::mouseReleaseEvent(e);
}

void ColorButton::mouseMoveEvent(QMouseEvent *e)
{
	if (!mMousePressed)
	{
		return;
	}

	if ((mPressPos - e->pos()).manhattanLength() > QApplication::startDragDistance())
	{
		mMousePressed = false;
		setDown(false);
		QDrag *drag = new QDrag(this);
		QMimeData *data = new QMimeData;
		data->setColorData(color());
		drag->setMimeData(data);
		drag->start(Qt::CopyAction);
	}
}

void ColorButton::paintEvent(QPaintEvent *)
{
	QPainter p(this);
	drawButton(&p);
}