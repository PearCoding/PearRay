#include "ImageView.h"
#include "exr/EXRFile.h"

#include <QMouseEvent>
#include <QPainter>
#include <QWheelEvent>

#include <QMenu>

constexpr int BAR_HEIGHT = 20;
constexpr int BAR_POS_W  = 100;
constexpr int MIN_S		 = 5;

constexpr float PAN_W  = 0.4f;
constexpr float ZOOM_W = 1.05f;

constexpr int GRID_S = 10;

ImageView::ImageView(QWidget* parent)
	: QWidget(parent)
	, mZoom(1)
	, mChannelMask(0xFF)
	, mLastPixel(0, 0)
{
	setContextMenuPolicy(Qt::CustomContextMenu);
	connect(this, SIGNAL(customContextMenuRequested(const QPoint&)),
			this, SLOT(showContextMenu(const QPoint&)));

	connect(&mSignalMapper, SIGNAL(mapped(QObject*)),
			this, SLOT(onContextMenuClick(QObject*)));

	setMouseTracking(true);
}

ImageView::~ImageView()
{
}

void ImageView::setLayer(const std::shared_ptr<EXRLayer>& layer)
{
	mChannelMask = 0xFF;
	mEXRLayer	= layer;
	updateImage();
}

QSize ImageView::minimumSizeHint() const
{
	return QSize(MIN_S, BAR_HEIGHT + MIN_S);
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

	// Value Field
	QString valS = valueAt(mLastPixel);
	painter.drawText(QRect(BAR_POS_W, 0, width() - BAR_POS_W - 1, BAR_HEIGHT - 1),
					 valS, QTextOption(Qt::AlignCenter));

	// Image
	painter.setClipRect(0, BAR_HEIGHT, width(), height() - BAR_HEIGHT);
	painter.setClipping(true);

	int dx = (width() - mPixmap.width()) / 2 + mDelta.x();
	int dy = (height() - mPixmap.height() - BAR_HEIGHT) / 2 + mDelta.y();
	painter.drawPixmap(dx, dy + BAR_HEIGHT, mPixmap);
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
	painter.drawLine(BAR_POS_W / 2, 0, BAR_POS_W / 2, BAR_HEIGHT - 1);
	painter.drawLine(BAR_POS_W, 0, BAR_POS_W, BAR_HEIGHT - 1);

	// Background
	const size_t gx = (mBackground.width() / (2 * GRID_S) + 1);
	const size_t gy = (mBackground.height() - BAR_HEIGHT) / GRID_S + 1;

	painter.setPen(Qt::NoPen);
	for (size_t y = 0; y < gy; ++y) {
		int dx = (y % 2) ? 0 : GRID_S;
		int py = BAR_HEIGHT + y * GRID_S;

		painter.setBrush(QBrush(Qt::white));
		for (size_t x = 0; x < gx; ++x)
			painter.drawRect(dx + x * GRID_S * 2, py,
							 GRID_S, GRID_S);

		painter.setBrush(QBrush(Qt::lightGray));
		for (size_t x = 0; x < gx; ++x)
			painter.drawRect(GRID_S - dx + x * GRID_S * 2, py,
							 GRID_S, GRID_S);
	}
	painter.end();
}

void ImageView::mousePressEvent(QMouseEvent* event)
{
	if (event->buttons() & Qt::LeftButton) {
		mLastPos = event->globalPos();
	}
}

void ImageView::mouseMoveEvent(QMouseEvent* event)
{
	if (event->buttons() & Qt::LeftButton) {
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
	mPixmap = QPixmap::fromImage(mImage.scaled(width() * mZoom,
											   (height() - BAR_HEIGHT) * mZoom,
											   Qt::KeepAspectRatio,
											   Qt::FastTransformation));
}

void ImageView::showContextMenu(const QPoint& p)
{
	if (!mEXRLayer)
		return;

	QMenu contextMenu(tr("Channels"), this);

	for (size_t i = 0; i < (size_t)mEXRLayer->data().size(); ++i) {
		QAction* action = new QAction(
			mEXRLayer->channelNames()[i],
			this);
		action->setCheckable(true);
		action->setChecked(mChannelMask & (1 << i));
		action->setData(QVariant::fromValue(i));

		connect(action, SIGNAL(triggered()), &mSignalMapper, SLOT(map()));

		mSignalMapper.setMapping(action, action);
		contextMenu.addAction(action);
	}

	contextMenu.exec(mapToGlobal(p));
}

void ImageView::onContextMenuClick(QObject* obj)
{
	QAction* act = qobject_cast<QAction*>(obj);
	if (!act)
		return;

	size_t id = act->data().toUInt();

	quint8 oldMask = mChannelMask;
	if (act->isChecked())
		mChannelMask |= (1 << id);
	else
		mChannelMask &= ~(1 << id);

	if (oldMask != mChannelMask)
		updateImage();
}

void ImageView::updateImage()
{
	if (mEXRLayer) {
		mEXRLayer->fillImage(mImage, mChannelMask);
	}
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
	if (mEXRLayer) {
		for (size_t i = 0; i < (size_t)mEXRLayer->data().size(); ++i) {
			float v = mEXRLayer->value(pixel.x(), pixel.y(), i);
			str += QString::number(v, 'g', 2) + " ";
		}
	}

	return str;
}