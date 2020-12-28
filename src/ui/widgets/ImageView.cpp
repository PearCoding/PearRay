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
	, mChannelMask(0xFF)
	, mChannelOffset(0)
	, mLastPixel(0, 0)
{
	zoomToOriginal();

	setContextMenuPolicy(Qt::CustomContextMenu);
	setMouseTracking(true);

	connect(this, &ImageView::customContextMenuRequested,
			this, &ImageView::showContextMenu);
}

ImageView::~ImageView()
{
}

void ImageView::zoomToFit()
{
	if (mImage.isNull())
		return;

	// Scale
	const float zw = viewSize().width() / (float)mImage.width();
	const float zh = viewSize().height() / (float)mImage.height();

	const float zoom = zw < zh ? zw : zh;

	mTransform = QTransform::fromScale(zoom, zoom);

	// Shift to center
	QPoint imgCenter   = mTransform.map(QPoint(mImage.width() / 2, mImage.height() / 2));
	QPoint viewCenter  = QPoint(viewSize().width() / 2, viewSize().height() / 2);
	QPoint deltaCenter = viewCenter - imgCenter;

	mTransform.translate(deltaCenter.x(), deltaCenter.y());
	mInvTransform = mTransform.inverted();

	repaint();
}

void ImageView::zoomToOriginal()
{
	if (mImage.isNull())
		return;
	// No scale

	// Shift to center
	QPoint imgCenter   = QPoint(mImage.width() / 2, mImage.height() / 2);
	QPoint viewCenter  = QPoint(viewSize().width() / 2, viewSize().height() / 2);
	QPoint deltaCenter = viewCenter - imgCenter;

	mTransform	  = QTransform::fromTranslate(deltaCenter.x(), deltaCenter.y());
	mInvTransform = mTransform.inverted();

	repaint();
}

QSize ImageView::viewSize() const
{
	return QSize(width(), height() - BAR_HEIGHT);
}

void ImageView::setView(const std::shared_ptr<ImageBufferView>& view)
{
	mChannelMask   = 0xFF;
	mChannelOffset = 0;
	mView		   = view;
	updateImage();
	updateGeometry();
}

void ImageView::setPipeline(const ImagePipeline& pipeline)
{
	mPipeline = pipeline;
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
	painter.setRenderHint(QPainter::SmoothPixmapTransform, false);
	painter.setRenderHint(QPainter::TextAntialiasing, true);
	painter.setRenderHint(QPainter::Antialiasing, true);

	painter.drawPixmap(0, 0, mBackground);

	// Position Field
	painter.setBrush(QBrush(Qt::white));
	painter.drawText(QRect(0, 0, BAR_POS_W / 2 - 1, BAR_HEIGHT - 1),
					 QString::number(mLastPixel.x()),
					 QTextOption(Qt::AlignCenter));
	painter.drawText(QRect(BAR_POS_W / 2 + 1, 0, BAR_POS_W / 2 - 1, BAR_HEIGHT - 1),
					 QString::number(mLastPixel.y()),
					 QTextOption(Qt::AlignCenter));

	const bool additionalField = mView->viewChannelCount() == 1;
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
	painter.translate(0, BAR_HEIGHT);
	painter.setTransform(mTransform, true);

	painter.drawPixmap(0, 0, mPixmap);

	event->accept();
}

// Cache background
void ImageView::resizeEvent(QResizeEvent* event)
{
	renderBackground(event->size());
}

void ImageView::renderBackground(const QSize& size)
{
	QPainter painter;
	mBackground = QPixmap(size);

	painter.begin(&mBackground);
	painter.setRenderHint(QPainter::Antialiasing, true);

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
		QPointF d = event->globalPos() - mLastPos;

		// No rotation involved, thats why this is fine.
		const float sx = mTransform.m11();
		const float sy = mTransform.m22();

		mTransform.translate(d.x() * PAN_W / sx, d.y() * PAN_W / sy);
		mInvTransform = mTransform.inverted();
		mLastPos	  = event->globalPos();
	}

	QPoint pixel = mapToPixel(event->pos());
	if (isValidPixel(pixel))
		mLastPixel = pixel;

	repaint();
}

void ImageView::wheelEvent(QWheelEvent* event)
{
	const float delta = event->angleDelta().y();
	if (std::abs(delta) < 1e-3f)
		return;

	const QPointF pos = mInvTransform.map(event->posF());
	const float zoom  = delta < 0 ? 1 / ZOOM_W : ZOOM_W;

	mTransform.translate(pos.x(), pos.y());
	mTransform.scale(zoom, zoom);
	mTransform.translate(-pos.x(), -pos.y());
	mInvTransform = mTransform.inverted();

	event->accept();

	repaint();
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
	mView->fillImage(mImage, mPipeline, mChannelOffset, mChannelMask);
	mPixmap = QPixmap::fromImage(mImage);

	repaint();
}

QPoint ImageView::mapToPixel(const QPoint& pos) const
{
	const QPointF gp = QPointF(pos.x(), pos.y() - BAR_HEIGHT);
	const QPointF lp = mInvTransform.map(gp);
	return QPoint(std::floor(lp.x()), std::floor(lp.y()));
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