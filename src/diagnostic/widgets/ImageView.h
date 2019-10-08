#pragma once

#include <QWidget>
#include <QSignalMapper>
#include <memory>

class EXRLayer;
class ImageView : public QWidget {
	Q_OBJECT
public:
	ImageView(QWidget* parent = nullptr);
	virtual ~ImageView();

	void setLayer(const std::shared_ptr<EXRLayer>& layer);

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

    QPoint mLastPos;
    QPoint mLastPixel;

	QImage mImage;
	QPixmap mPixmap;
	QPixmap mBackground;

	std::shared_ptr<EXRLayer> mEXRLayer;

    QSignalMapper mSignalMapper;
};