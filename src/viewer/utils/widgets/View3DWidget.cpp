#include "View3DWidget.h"

#include <QOpenGLContext>
#include <QOpenGLFunctions>

#include <QWheelEvent>

View3DWidget::View3DWidget(QWidget* parent)
	: QOpenGLWidget(parent)
	, mLastMode(IM_ROTATE)
{
}

View3DWidget::~View3DWidget()
{
}

void View3DWidget::addGraphicObject(const std::shared_ptr<GraphicObject>& obj)
{
	if (obj)
		mObjects.append(obj);
}

void View3DWidget::clear()
{
	for (auto obj : mObjects)
		obj->clear();
}

void View3DWidget::rebuild()
{
	makeCurrent();

	for (auto obj : mObjects)
		obj->rebuild();

	//mCamera.setZoom(1);
	//mCamera.setEyePosition(QVector3D(0, 0, container.lowerBound(2)));
	//mCamera.setRotation(QQuaternion());

	update();
}

void View3DWidget::initializeGL()
{
	for (auto obj : mObjects)
		obj->initializeGL();
}

void View3DWidget::paintGL()
{
	const QMatrix4x4 WV = mProjection * mCamera.getViewMatrix();
	for (auto obj : mObjects)
		obj->paintGL(WV);
}

void View3DWidget::resizeGL(int w, int h)
{
	QOpenGLFunctions* f		= QOpenGLContext::currentContext()->functions();
	const qreal retinaScale = devicePixelRatio();

	f->glViewport(0, 0, w * retinaScale, h * retinaScale);
	mProjection.setToIdentity();
	mProjection.perspective(60.0f, retinaScale, 0.1f, 10000.0f);
}

void View3DWidget::mousePressEvent(QMouseEvent* event)
{
	if (event->buttons() & Qt::LeftButton) {
		mLastPoint = event->localPos();
		mLastMode  = IM_ROTATE;
		event->accept();
	} else if (event->buttons() & Qt::MiddleButton) {
		mLastPoint = event->localPos();
		mLastMode  = IM_PAN;
		event->accept();
	}
}

constexpr float RS = 0.1f;
void View3DWidget::mouseMoveEvent(QMouseEvent* event)
{
	if (mLastMode == IM_ROTATE && event->buttons() & Qt::LeftButton) {
		QPointF delta = event->localPos() - mLastPoint;
		mLastPoint	= event->localPos();

		QQuaternion dt = QQuaternion::fromEulerAngles(delta.y() * RS, delta.x() * RS, 0);
		mCamera.setRotation(mCamera.rotation() * dt);

		update();
		event->accept();
	} else if (mLastMode == IM_PAN && event->buttons() & Qt::MiddleButton) {
		QPointF delta = event->localPos() - mLastPoint;
		mLastPoint	= event->localPos();

		mCamera.pan(delta * 0.001f);

		update();
		event->accept();
	}
}

constexpr float DT = 0.8f;
void View3DWidget::wheelEvent(QWheelEvent* event)
{
	const float delta = event->delta();
	if (delta > 0) {
		mCamera.setZoom(mCamera.zoom() / DT);
	} else if (delta < 0) {
		mCamera.setZoom(mCamera.zoom() * DT);
	}

	event->accept();
	update();
}