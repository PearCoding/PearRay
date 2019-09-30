#include "View3DWidget.h"

#include <QOpenGLContext>
#include <QOpenGLFunctions>

#include <QWheelEvent>

View3DWidget::View3DWidget(QWidget* parent)
	: QOpenGLWidget(parent)
	, mProgram(nullptr)
	, mVertexBuffer(nullptr)
	, mIndexBuffer(nullptr)
	, mVAO(nullptr)
{
}

View3DWidget::~View3DWidget()
{
	if (mProgram)
		delete mProgram;

	if (mVertexBuffer)
		delete mVertexBuffer;

	if (mIndexBuffer)
		delete mIndexBuffer;
}

void View3DWidget::clear()
{
	mVertices.clear();
	mIndices.clear();
}

void View3DWidget::rebuild()
{
	makeCurrent();

	// VAO
	if (mVAO)
		delete mVAO;
	mVAO = new QOpenGLVertexArrayObject();
	if (!mVAO->create()) {
		qCritical() << "Couldn't create VAO";
		return;
	}

	if (mVertexBuffer)
		delete mVertexBuffer;
	mVertexBuffer = new QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
	if (!mVertexBuffer->create()) {
		qCritical() << "Couldn't create vertex buffer";
		return;
	}

	if (mIndexBuffer)
		delete mIndexBuffer;
	mIndexBuffer = new QOpenGLBuffer(QOpenGLBuffer::IndexBuffer);
	if (!mIndexBuffer->create()) {
		qCritical() << "Couldn't create index buffer";
		return;
	}

	// Setup
	mVAO->bind();
	mProgram->bind();

	mVertexBuffer->setUsagePattern(QOpenGLBuffer::StaticDraw);
	mVertexBuffer->bind();
	mVertexBuffer->allocate(mVertices.data(), mVertices.size() * sizeof(QVector3D));
	mProgram->enableAttributeArray(0);
	mProgram->setAttributeBuffer(0, GL_FLOAT, 0, 3);

	mIndexBuffer->setUsagePattern(QOpenGLBuffer::StaticDraw);
	mIndexBuffer->bind();
	mIndexBuffer->allocate(mIndices.data(), mIndices.size() * sizeof(unsigned int));

	mProgram->release();
	mVAO->release();

	mVertexBuffer->release();
	mIndexBuffer->release();

	//mCamera.setZoom(1);
	//mCamera.setEyePosition(QVector3D(0, 0, container.lowerBound(2)));
	//mCamera.setRotation(QQuaternion());

	update();
}

static const char* vertexShaderSource = "attribute highp vec4 posAttr;\n"
										"uniform highp mat4 matrix;\n"
										"void main() {\n"
										"   gl_Position = matrix * posAttr;\n"
										"}\n";

static const char* fragmentShaderSource = "void main() {\n"
										  "   gl_FragColor = vec4(1,1,1,1);\n"
										  "}\n";

void View3DWidget::initializeGL()
{
	QOpenGLFunctions* f = QOpenGLContext::currentContext()->functions();
	f->glClearColor(0.1f, 0.1f, 0.1f, 1.0f);

	mProgram = new QOpenGLShaderProgram(this);
	mProgram->addShaderFromSourceCode(QOpenGLShader::Vertex, vertexShaderSource);
	mProgram->addShaderFromSourceCode(QOpenGLShader::Fragment, fragmentShaderSource);
	mProgram->link();

	mPosAttr	   = mProgram->attributeLocation("posAttr");
	mMatrixUniform = mProgram->uniformLocation("matrix");
}

void View3DWidget::paintGL()
{
	QOpenGLFunctions* f = QOpenGLContext::currentContext()->functions();
	f->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	if (mVertices.empty())
		return;

	mProgram->bind();
	mVAO->bind();

	mProgram->setUniformValue(mMatrixUniform, mProjection * mCamera.getViewMatrix());

	f->glDrawElements(GL_LINES, mIndices.size(), GL_UNSIGNED_INT, 0);

	mProgram->release();
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
		event->accept();
	}
}

constexpr float RS = 0.1f;
void View3DWidget::mouseMoveEvent(QMouseEvent* event)
{
	if (event->buttons() & Qt::LeftButton) {
		QPointF delta = event->localPos() - mLastPoint;
		mLastPoint	= event->localPos();

		QQuaternion dt = QQuaternion::fromEulerAngles(delta.y() * RS, delta.x() * RS, 0);
		mCamera.setRotation(mCamera.rotation() * dt);

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