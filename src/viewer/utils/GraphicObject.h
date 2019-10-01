#pragma once

#include <QOpenGLBuffer>
#include <QOpenGLShaderProgram>
#include <QOpenGLVertexArrayObject>
#include <QVector>
#include <memory>

class GraphicObject {
public:
	GraphicObject();
	virtual ~GraphicObject();

	inline QVector<QVector3D>& vertices() { return mVertices; }
	inline QVector<unsigned int>& indices() { return mIndices; }
	inline QMatrix4x4& transform() { return mTransform; }

	inline GLuint drawMode() const { return mDrawMode; }
	inline void setDrawMode(GLuint mode) { mDrawMode = mode; }

	void clear();
	void rebuild();

	void initializeGL();
	void paintGL(const QMatrix4x4& worldView);

private:
	GLuint mPosAttr;
	GLuint mMatrixUniform;
	GLuint mDrawMode;

	std::unique_ptr<QOpenGLShaderProgram> mProgram;
	std::unique_ptr<QOpenGLBuffer> mVertexBuffer;
	std::unique_ptr<QOpenGLBuffer> mIndexBuffer;
	std::unique_ptr<QOpenGLVertexArrayObject> mVAO;

	QVector<QVector3D> mVertices;
	QVector<unsigned int> mIndices;

	QMatrix4x4 mTransform;
};