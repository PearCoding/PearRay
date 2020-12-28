#pragma once

#include "mapper/ImagePipeline.h"

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
	void setPipeline(const ImagePipeline& mapper);

	void exportImage(const QString& path) const;

	QSize minimumSizeHint() const override;
	QSize sizeHint() const override;

public slots:
	void zoomToFit();
	void zoomToOriginal();

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
	void renderBackground(const QSize& size);
	void updateImage();
	
	QSize viewSize() const;

	QPoint mapToPixel(const QPoint& pos) const;
	bool isValidPixel(const QPoint& pixel) const;
	QString valueAt(const QPoint& pixel) const;

	QTransform mTransform;
	QTransform mInvTransform;
	quint8 mChannelMask;
	quint32 mChannelOffset;

	QPoint mLastPos;
	QPoint mLastPixel;

	QImage mImage;
	QPixmap mPixmap;
	QPixmap mBackground;

	std::shared_ptr<ImageBufferView> mView;
	ImagePipeline mPipeline;
};
} // namespace UI
} // namespace PR