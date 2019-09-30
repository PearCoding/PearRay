#pragma once

#include <QOpenGLBuffer>
#include <QOpenGLShaderProgram>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLWidget>
#include <QVector>

#include "OrbitCamera.h"

class View3DWidget : public QOpenGLWidget {
	Q_OBJECT
public:
	View3DWidget(QWidget* parent = nullptr);
	virtual ~View3DWidget();

	inline QVector<QVector3D>& vertices() { return mVertices; }
	inline QVector<unsigned int>& indices() { return mIndices; }

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
	GLuint mPosAttr;
	GLuint mMatrixUniform;

	QOpenGLShaderProgram* mProgram;
	QOpenGLBuffer* mVertexBuffer;
	QOpenGLBuffer* mIndexBuffer;
	QOpenGLVertexArrayObject* mVAO;

	QVector<QVector3D> mVertices;
	QVector<unsigned int> mIndices;

	QMatrix4x4 mProjection;

	OrbitCamera mCamera;
	QPointF mLastPoint;
};