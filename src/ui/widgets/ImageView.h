#pragma once

#include "mapper/ToneMapper.h"

#include <QSignalMapper>
#include <QWidget>
#include <memory>

namespace PR {
namespace UI {
class ImageBufferView;
class PR_LIB_UI ImageView : public QWidget {
	Q_OBJECT
public:
	ImageView(QWidget* parent = nullptr);
	virtual ~ImageView();

	inline std::shared_ptr<ImageBufferView> view() const { return mView; }
	void setView(const std::shared_ptr<ImageBufferView>& view);
	void setMapper(const ToneMapper& mapper);

	void exportImage(const QString& path) const;

	QSize minimumSizeHint() const override;
	QSize sizeHint() const override;

public slots:
	void resetView();
	void zoomToOriginalSize();

protected:
	void paintEvent(QPaintEvent* event) override;
	void resizeEvent(QResizeEvent* event) override;
	void mousePressEvent(QMouseEvent* event) override;
	void mouseMoveEvent(QMouseEvent* event) override;
	void wheelEvent(QWheelEvent* event) override;

private slots:
	void showContextMenu(const QPoint& p);
	void onContextMenuClick(QObject* obj);

private:
	void updateImage();
	void cacheImage();

	QPoint mapToPixel(const QPoint& pos) const;
	bool isValidPixel(const QPoint& pixel) const;
	QString valueAt(const QPoint& pixel) const;

	float mZoom;
	QPointF mDelta;
	quint8 mChannelMask;
	quint32 mChannelOffset;

	QPoint mLastPos;
	QPoint mLastPixel;

	QImage mImage;
	QPixmap mPixmap;
	QPixmap mBackground;

	std::shared_ptr<ImageBufferView> mView;
	ToneMapper mMapper;
};
} // namespace UI
} // namespace PR