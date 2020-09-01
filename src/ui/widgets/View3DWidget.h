#pragma once

#include <QOpenGLWidget>
#include <QVector>
#include <memory>

#include "3d/GraphicObject.h"
#include "3d/OrbitCamera.h"

namespace PRUI {
class PR_LIB_UI View3DWidget : public QOpenGLWidget {
	Q_OBJECT
public:
	View3DWidget(QWidget* parent = nullptr);
	virtual ~View3DWidget();

	void addGraphicObject(const std::shared_ptr<GraphicObject>& obj);
	void removeGraphicObject(const std::shared_ptr<GraphicObject>& obj);

	void clear();
	void rebuild();

	void addAxis();
	void removeAxis();

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
	std::shared_ptr<GraphicObject> mAxis;

	QMatrix4x4 mProjection;

	OrbitCamera mCamera;
	QPointF mLastPoint;
	InteractionMode mLastMode;
};
}