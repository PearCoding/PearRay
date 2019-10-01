#pragma once

#include <QOpenGLWidget>
#include <QVector>
#include <memory>

#include "GraphicObject.h"
#include "OrbitCamera.h"

class View3DWidget : public QOpenGLWidget {
	Q_OBJECT
public:
	View3DWidget(QWidget* parent = nullptr);
	virtual ~View3DWidget();

	void addGraphicObject(const std::shared_ptr<GraphicObject>& obj);

	void clear();
	void rebuild();

protected:
	void initializeGL() override;
	void paintGL() override;
	void resizeGL(int w, int h) override;

	void mousePressEvent(QMouseEvent* event) override;
	void mouseMoveEvent(QMouseEvent* event) override;
	void wheelEvent(QWheelEvent* event) override;

private:
	enum InteractionMode {
		IM_ROTATE,
		IM_PAN
	};

	QVector<std::shared_ptr<GraphicObject>> mObjects;

	QMatrix4x4 mProjection;

	OrbitCamera mCamera;
	QPointF mLastPoint;
	InteractionMode mLastMode;
};