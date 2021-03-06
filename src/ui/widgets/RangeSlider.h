#pragma once

#include "PR_Config.h"
#include <QWidget>

namespace PR {
namespace UI {
class PR_LIB_UI RangeSlider : public QWidget {
	Q_OBJECT
public:
	RangeSlider(QWidget* parent = nullptr);
	RangeSlider(float min, float max, QWidget* parent = nullptr);
	virtual ~RangeSlider();

	QSize minimumSizeHint() const override;
	QSize sizeHint() const override;

	inline float leftValue() const { return mLeft; }
	inline float rightValue() const { return mRight; }
	inline float minValue() const { return mMin; }
	inline float maxValue() const { return mMax; }

public slots:
	void setLeftValue(float f);
	void setRightValue(float f);
	void setMinValue(float f);
	void setMaxValue(float f);

signals:
	void leftValueChanged(float);
	void rightValueChanged(float);

protected:
	void paintEvent(QPaintEvent* event) override;
	void mousePressEvent(QMouseEvent* event) override;
	void mouseMoveEvent(QMouseEvent* event) override;
	void wheelEvent(QWheelEvent* event) override;

#ifndef QT_NO_CONTEXTMENU
	void contextMenuEvent(QContextMenuEvent* event) override;
#endif // QT_NO_CONTEXTMENU

private:
	struct DrawStyle {
		int LeftKnobStart;
		int RightKnobStart;
		int SlideStart;
		int SlideEnd;
		QColor SlideBarColor;
		QColor SlideKnobColor;
		QColor TextColor;
	};
	DrawStyle calculateStyle() const;

	void pan(int dx);

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
} // namespace UI
} // namespace PR