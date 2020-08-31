#include "View3DWidget.h"

#include <QOpenGLContext>
#include <QOpenGLFunctions>

#include <QWheelEvent>

namespace PRUI {
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
	QOpenGLFunctions* f = QOpenGLContext::currentContext()->functions();
	f->glEnable(GL_DEPTH_TEST);
	f->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

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
		mLastPoint = event->globalPos();
		mLastMode  = IM_ROTATE;
		event->accept();
	} else if (event->buttons() & Qt::MiddleButton) {
		mLastPoint = event->globalPos();
		mLastMode  = IM_PAN;
		event->accept();
	}
}

constexpr float RS = 0.1f;
void View3DWidget::mouseMoveEvent(QMouseEvent* event)
{
	if (mLastMode == IM_ROTATE && event->buttons() & Qt::LeftButton) {
		QPointF delta = event->globalPos() - mLastPoint;
		mLastPoint	= event->globalPos();

		QQuaternion dt = QQuaternion::fromEulerAngles(delta.y() * RS, delta.x() * RS, 0);
		mCamera.setRotation(mCamera.rotation() * dt);

		update();
		event->accept();
	} else if (mLastMode == IM_PAN && event->buttons() & Qt::MiddleButton) {
		QPointF delta = event->globalPos() - mLastPoint;
		mLastPoint	= event->globalPos();

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

void View3DWidget::addAxis()
{
	std::shared_ptr<GraphicObject> axis = std::make_shared<GraphicObject>(true);

	axis->vertices().resize(6);
	axis->colors().resize(6);
	axis->indices().resize(6);

	axis->vertices()[0] = QVector3D(0, 0, 0);
	axis->vertices()[1] = QVector3D(1, 0, 0);
	axis->vertices()[2] = QVector3D(0, 0, 0);
	axis->vertices()[3] = QVector3D(0, 1, 0);
	axis->vertices()[4] = QVector3D(0, 0, 0);
	axis->vertices()[5] = QVector3D(0, 0, 1);

	axis->colors()[0] = QVector3D(1, 0, 0);
	axis->colors()[1] = QVector3D(1, 0, 0);
	axis->colors()[2] = QVector3D(0, 1, 0);
	axis->colors()[3] = QVector3D(0, 1, 0);
	axis->colors()[4] = QVector3D(0, 0, 1);
	axis->colors()[5] = QVector3D(0, 0, 1);

	for (int i = 0; i < 6; ++i)
		axis->indices()[i] = i;

	addGraphicObject(axis);
}
}