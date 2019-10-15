#pragma once

#include <QWidget>

class RangeSlider : public QWidget {
	Q_OBJECT
public:
	RangeSlider(QWidget* parent = nullptr);
	RangeSlider(float min, float max, QWidget* parent = nullptr);
	virtual ~RangeSlider();

	QSize minimumSizeHint() const override;
	QSize sizeHint() const override;

public slots:
	void setLeft(float f);
	void setRight(float f);
	void setMin(float f);
	void setMax(float f);

	void fitFull();

signals:
	void leftChanged(float);
	void rightChanged(float);

protected:
	void paintEvent(QPaintEvent* event) override;
	void mousePressEvent(QMouseEvent* event) override;
	void mouseMoveEvent(QMouseEvent* event) override;
	void wheelEvent(QWheelEvent* event) override;

private:
	struct DrawStyle {
		quint32 LeftKnobStart;
		quint32 RightKnobStart;
		quint32 SlideStart;
		quint32 SlideEnd;
	};
	DrawStyle calculateStyle() const;

	float mLeft;
	float mRight;
	float mMin;
	float mMax;

	enum MouseOperation {
		MO_NONE,
		MO_KNOB_LEFT,
		MO_KNOB_RIGHT,
		MO_PAN
	};
	MouseOperation mCurrentOperation;
	int mLastX;
	int mPickDelta;
};