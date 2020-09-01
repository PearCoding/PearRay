#include "ImageView.h"
#include "io/ImageBufferView.h"

#include <QMouseEvent>
#include <QPainter>
#include <QWheelEvent>

#include <QMenu>

namespace PR {
namespace UI {
constexpr int BAR_HEIGHT = 20;
constexpr int BAR_POS_W	 = 100;
constexpr int MIN_S		 = 400;

constexpr float PAN_W  = 0.4f;
constexpr float ZOOM_W = 1.05f;

constexpr int GRID_S = 10;

ImageView::ImageView(QWidget* parent)
	: QWidget(parent)
	, mZoom(1)
	, mChannelMask(0xFF)
	, mChannelOffset(0)
	, mLastPixel(0, 0)
{
	setContextMenuPolicy(Qt::CustomContextMenu);
	connect(this, SIGNAL(customContextMenuRequested(const QPoint&)),
			this, SLOT(showContextMenu(const QPoint&)));

	setMouseTracking(true);
}

ImageView::~ImageView()
{
}

void ImageView::resetView()
{
	float zw = width() / (float)mImage.width();
	float zh = (height() - BAR_HEIGHT) / (float)mImage.height();

	mZoom  = std::min(zw, zh);
	mDelta = QPointF(0, 0);

	cacheImage();
	repaint();
}

void ImageView::zoomToOriginalSize()
{
	mZoom  = 1;
	mDelta = QPointF(0, 0);

	cacheImage();
	repaint();
}

void ImageView::setView(const std::shared_ptr<ImageBufferView>& view)
{
	mChannelMask   = 0xFF;
	mChannelOffset = 0;
	mView		   = view;
	updateImage();
	updateGeometry();
}

void ImageView::setMapper(const ToneMapper& mapper)
{
	mMapper = mapper;
	updateImage();
}

void ImageView::exportImage(const QString& path) const
{
	mImage.save(path);
}

QSize ImageView::minimumSizeHint() const
{
	if (!mImage.isNull()) {
		return QSize(qMin(mImage.width(), MIN_S), BAR_HEIGHT + qMin(mImage.height(), MIN_S));
	} else {
		return QSize(MIN_S, BAR_HEIGHT + MIN_S);
	}
}

QSize ImageView::sizeHint() const
{
	if (!mImage.isNull()) {
		return QSize(mImage.width(), BAR_HEIGHT + mImage.height());
	} else {
		return minimumSizeHint();
	}
}

void ImageView::paintEvent(QPaintEvent* event)
{
	QPainter painter(this);
	painter.drawPixmap(0, 0, mBackground);

	// Position Field
	painter.setBrush(QBrush(Qt::white));
	painter.drawText(QRect(0, 0, BAR_POS_W / 2 - 1, BAR_HEIGHT - 1),
					 QString::number(mLastPixel.x()),
					 QTextOption(Qt::AlignCenter));
	painter.drawText(QRect(BAR_POS_W / 2 + 1, 0, BAR_POS_W / 2 - 1, BAR_HEIGHT - 1),
					 QString::number(mLastPixel.y()),
					 QTextOption(Qt::AlignCenter));

	bool additionalField = mView->viewChannelCount() == 1;
	if (additionalField) {
		painter.drawText(QRect(BAR_POS_W + 1, 0, BAR_POS_W / 2 - 1, BAR_HEIGHT - 1),
						 QString::number(mChannelOffset),
						 QTextOption(Qt::AlignCenter));

		painter.drawLine(BAR_POS_W + BAR_POS_W / 2, 0,
						 BAR_POS_W + BAR_POS_W / 2, BAR_HEIGHT - 1);
	}

	// Value Field
	QString valS = valueAt(mLastPixel);
	painter.drawText(QRect(BAR_POS_W + (additionalField ? BAR_POS_W / 2 : 0), 0,
						   width() - BAR_POS_W - 1, BAR_HEIGHT - 1),
					 valS, QTextOption(Qt::AlignCenter));

	// Image
	painter.setClipRect(0, BAR_HEIGHT, width(), height() - BAR_HEIGHT);
	painter.setClipping(true);

	int dx = (width() - mPixmap.width()) / 2 + mDelta.x();
	int dy = (height() - mPixmap.height() - BAR_HEIGHT) / 2 + mDelta.y();
	painter.drawPixmap(dx, dy + BAR_HEIGHT, mPixmap);

	event->accept();
}

// Cache background
void ImageView::resizeEvent(QResizeEvent* event)
{
	QPainter painter;
	mBackground = QPixmap(event->size());

	painter.begin(&mBackground);

	// Bar
	painter.setBrush(QBrush(Qt::darkGray));
	painter.drawRect(0, 0, mBackground.width() - 1, BAR_HEIGHT - 1);

	// Separators
	painter.drawLine(BAR_POS_W / 2, 0,
					 BAR_POS_W / 2, BAR_HEIGHT - 1);
	painter.drawLine(BAR_POS_W, 0,
					 BAR_POS_W, BAR_HEIGHT - 1);

	// Background
	const int gx = (mBackground.width() / (2 * GRID_S) + 1);
	const int gy = (mBackground.height() - BAR_HEIGHT) / GRID_S + 1;

	painter.setPen(Qt::NoPen);
	for (int y = 0; y < gy; ++y) {
		int dx = (y % 2) ? 0 : GRID_S;
		int py = BAR_HEIGHT + y * GRID_S;

		painter.setBrush(QBrush(Qt::white));
		for (int x = 0; x < gx; ++x)
			painter.drawRect(dx + x * GRID_S * 2, py,
							 GRID_S, GRID_S);

		painter.setBrush(QBrush(Qt::lightGray));
		for (int x = 0; x < gx; ++x)
			painter.drawRect(GRID_S - dx + x * GRID_S * 2, py,
							 GRID_S, GRID_S);
	}
	painter.end();
}

void ImageView::mousePressEvent(QMouseEvent* event)
{
	if ((event->buttons() & Qt::LeftButton)
		|| (event->buttons() & Qt::MiddleButton)) {
		mLastPos = event->globalPos();
	}
}

void ImageView::mouseMoveEvent(QMouseEvent* event)
{
	if ((event->buttons() & Qt::LeftButton)
		|| (event->buttons() & Qt::MiddleButton)) {
		QPointF d = mLastPos - event->globalPos();

		mDelta += -d * PAN_W;
		mLastPos = event->globalPos();
	}

	QPoint pixel = mapToPixel(event->pos());
	if (isValidPixel(pixel))
		mLastPixel = pixel;

	repaint();
}

void ImageView::wheelEvent(QWheelEvent* event)
{
	float delta = event->angleDelta().y();

	if (delta < 0)
		mZoom /= ZOOM_W;
	else if (delta > 0)
		mZoom *= ZOOM_W;

	event->accept();

	cacheImage();
	repaint();
}

void ImageView::cacheImage()
{
	mPixmap = QPixmap::fromImage(mImage.scaled(mImage.width() * mZoom,
											   mImage.height() * mZoom,
											   Qt::KeepAspectRatio,
											   Qt::FastTransformation));
}

void ImageView::showContextMenu(const QPoint& p)
{
	QMenu contextMenu(tr("Channels"), this);

	if (mView->viewChannelCount() != 1) {
		for (int i = 0; i < mView->channelCount(); ++i) {
			QAction* action = new QAction(
				mView->channelName(i),
				this);
			action->setCheckable(true);
			action->setChecked(mChannelMask & (1 << i));
			action->setData(QVariant::fromValue(i));

			connect(action, &QAction::triggered, [&]() { this->onContextMenuClick(this->sender()); });
			contextMenu.addAction(action);
		}
	} else {
		for (int i = 0; i < mView->channelCount(); ++i) {
			QAction* action = new QAction(
				mView->channelName(i),
				this);
			action->setData(QVariant::fromValue(i));

			connect(action, &QAction::triggered, [&]() { this->onContextMenuClick(this->sender()); });

			contextMenu.addAction(action);
		}
	}

	contextMenu.exec(mapToGlobal(p));
}

void ImageView::onContextMenuClick(QObject* obj)
{
	QAction* act = qobject_cast<QAction*>(obj);
	if (!act)
		return;

	uint id = act->data().toUInt();

	quint8 oldMask = mChannelMask;
	quint32 oldOff = mChannelOffset;
	if (mView->viewChannelCount() != 1) {
		if (act->isChecked())
			mChannelMask |= (1 << id);
		else
			mChannelMask &= ~(1 << id);
	} else {
		mChannelOffset = id;
	}

	if (oldMask != mChannelMask
		|| oldOff != mChannelOffset)
		updateImage();
}

void ImageView::updateImage()
{
	mView->fillImage(mImage, mMapper, mChannelOffset, mChannelMask);
	cacheImage();
	repaint();
}

QPoint ImageView::mapToPixel(const QPoint& pos) const
{
	int dx = (width() - mPixmap.width()) / 2 + mDelta.x();
	int dy = (height() - mPixmap.height() - BAR_HEIGHT) / 2 + mDelta.y() + BAR_HEIGHT;

	const float sx = mImage.width() / (float)mPixmap.width();
	const float sy = mImage.height() / (float)mPixmap.height();

	return QPoint((pos.x() - dx) * sx,
				  (pos.y() - dy) * sy);
}

bool ImageView::isValidPixel(const QPoint& pixel) const
{
	return pixel.x() >= 0 && pixel.x() < mImage.width()
		   && pixel.y() >= 0 && pixel.y() < mImage.height();
}

QString ImageView::valueAt(const QPoint& pixel) const
{
	QString str;
	for (int i = 0; i < mView->channelCount(); ++i) {
		float v = mView->value(pixel.x(), pixel.y(), i);
		str += QString::number(v, 'g', 2) + " ";
	}

	return str;
}
} // namespace UI
} // namespace PR