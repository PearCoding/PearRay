#pragma once

#include <QPushButton>

class ColorButton : public QPushButton {
	Q_OBJECT

public:
	ColorButton(QWidget* parent);
	~ColorButton();

	void setFlat(bool b);
	bool isFlat() const;

	const QColor& color() const;
	void setColor(const QColor&);
	QSize sizeHint() const;
	QSize minimumSizeHint() const;
	void mousePressEvent(QMouseEvent*);
	void mouseReleaseEvent(QMouseEvent*);
	void mouseMoveEvent(QMouseEvent*);
	void dragEnterEvent(QDragEnterEvent*);
	void dragMoveEvent(QDragMoveEvent*);
	void dropEvent(QDropEvent*);

signals:
	void colorChanged(const QColor&);

protected:
	void paintEvent(QPaintEvent*);
	void drawButton(QPainter*);
	void drawButtonLabel(QPainter*);

private slots:
	void changeColor();

private:
	QColor mColor;
	QPoint mPressPos;
	bool mMousePressed;
	bool mIsFlat;
};
