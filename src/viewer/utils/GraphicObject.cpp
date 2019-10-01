#include "GraphicObject.h"

#include <QOpenGLContext>
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>

GraphicObject::GraphicObject()
	: mPosAttr(0)
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
	mIndices.clear();
}

void GraphicObject::rebuild()
{
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

void GraphicObject::initializeGL()
{
	QOpenGLFunctions* f = QOpenGLContext::currentContext()->functions();
	f->glClearColor(0.1f, 0.1f, 0.1f, 1.0f);

	mProgram = std::make_unique<QOpenGLShaderProgram>();
	mProgram->addShaderFromSourceCode(QOpenGLShader::Vertex, vertexShaderSource);
	mProgram->addShaderFromSourceCode(QOpenGLShader::Fragment, fragmentShaderSource);
	mProgram->link();

	mPosAttr	   = mProgram->attributeLocation("posAttr");
	mMatrixUniform = mProgram->uniformLocation("matrix");
}

void GraphicObject::paintGL(const QMatrix4x4& worldView)
{
	if(!mVAO)
		return;

	if (!mProgram)
		initializeGL();

	QOpenGLFunctions* f = QOpenGLContext::currentContext()->functions();
	f->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	if (mVertices.empty())
		return;

	mProgram->bind();
	mVAO->bind();

	mProgram->setUniformValue(mMatrixUniform, worldView * mTransform);

	f->glDrawElements(mDrawMode, mIndices.size(), GL_UNSIGNED_INT, 0);

	mProgram->release();
}