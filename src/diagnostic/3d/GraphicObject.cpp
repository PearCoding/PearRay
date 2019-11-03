#include "GraphicObject.h"

#include <QOpenGLContext>
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>

GraphicObject::GraphicObject(bool useColor)
	: mUseColor(useColor)
	, mPosAttr(0)
	, mColAttr(0)
	, mMatrixUniform(0)
	, mDrawMode(GL_LINES)
{
	mTransform.setToIdentity();
}

GraphicObject::~GraphicObject()
{
}

void GraphicObject::clear()
{
	mVertices.clear();
	mColors.clear();
	mIndices.clear();
}

void GraphicObject::rebuild()
{
	if(!mProgram)
		initializeGL();

	// VAO
	mVAO = std::make_unique<QOpenGLVertexArrayObject>();
	if (!mVAO->create()) {
		qCritical() << "Couldn't create VAO";
		return;
	}

	mVertexBuffer = std::make_unique<QOpenGLBuffer>(QOpenGLBuffer::VertexBuffer);
	if (!mVertexBuffer->create()) {
		qCritical() << "Couldn't create vertex buffer";
		return;
	}

	mColorVertexBuffer = std::make_unique<QOpenGLBuffer>(QOpenGLBuffer::VertexBuffer);
	if (!mColorVertexBuffer->create()) {
		qCritical() << "Couldn't create vertex buffer";
		return;
	}

	mIndexBuffer = std::make_unique<QOpenGLBuffer>(QOpenGLBuffer::IndexBuffer);
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

	if (mUseColor) {
		mColorVertexBuffer->setUsagePattern(QOpenGLBuffer::StaticDraw);
		mColorVertexBuffer->bind();
		mColorVertexBuffer->allocate(mColors.data(), mColors.size() * sizeof(QVector3D));
		mProgram->enableAttributeArray(1);
		mProgram->setAttributeBuffer(1, GL_FLOAT, 0, 3);
	}

	mIndexBuffer->setUsagePattern(QOpenGLBuffer::StaticDraw);
	mIndexBuffer->bind();
	mIndexBuffer->allocate(mIndices.data(), mIndices.size() * sizeof(unsigned int));

	mProgram->release();
	mVAO->release();

	mVertexBuffer->release();
	mIndexBuffer->release();
}

static const char* vertexShaderSource = "attribute highp vec4 posAttr;\n"
										"uniform highp mat4 matrix;\n"
										"void main() {\n"
										"   gl_Position = matrix * posAttr;\n"
										"}\n";

static const char* fragmentShaderSource = "void main() {\n"
										  "   gl_FragColor = vec4(1,1,1,1);\n"
										  "}\n";

static const char* vertexShaderSource2 = "attribute vec4 posAttr;\n"
										 "attribute vec4 colAttr;\n"
										 "uniform highp mat4 matrix;\n"
										 "varying vec4 color;\n"
										 "void main() {\n"
										 "   gl_Position = matrix * posAttr;\n"
										 "   color = colAttr;\n"
										 "}\n";

static const char* fragmentShaderSource2 = "varying vec4 color;\n"
										   "void main() {\n"
										   "   gl_FragColor = color;\n"
										   "}\n";

void GraphicObject::initializeGL()
{
	QOpenGLFunctions* f = QOpenGLContext::currentContext()->functions();
	f->glClearColor(0.1f, 0.1f, 0.1f, 1.0f);

	mProgram = std::make_unique<QOpenGLShaderProgram>();
	if (mUseColor) {
		mProgram->addShaderFromSourceCode(QOpenGLShader::Vertex, vertexShaderSource2);
		mProgram->addShaderFromSourceCode(QOpenGLShader::Fragment, fragmentShaderSource2);
	} else {
		mProgram->addShaderFromSourceCode(QOpenGLShader::Vertex, vertexShaderSource);
		mProgram->addShaderFromSourceCode(QOpenGLShader::Fragment, fragmentShaderSource);
	}
	mProgram->link();

	mPosAttr	   = mProgram->attributeLocation("posAttr");
	mColAttr	   = mProgram->attributeLocation("colAttr");
	mMatrixUniform = mProgram->uniformLocation("matrix");
}

void GraphicObject::paintGL(const QMatrix4x4& worldView)
{
	if (!mVAO)
		return;

	QOpenGLFunctions* f = QOpenGLContext::currentContext()->functions();

	if (mVertices.empty())
		return;

	mProgram->bind();
	mVAO->bind();

	mProgram->setUniformValue(mMatrixUniform, worldView * mTransform);

	f->glDrawElements(mDrawMode, mIndices.size(), GL_UNSIGNED_INT, 0);

	mProgram->release();
}