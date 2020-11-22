#pragma once

#include <QOpenGLWidget>
#include <QVector>

#include "3d/Camera.h"

namespace PR {
namespace UI {
class GraphicEntity;
class Axis3DEntity;
class GridEntity;
class ScreenSpaceEntity;
struct ShadingContext;

// In increasing priority order
enum ViewLayer {
	VL_Background4 = 0,
	VL_Background3,
	VL_Background2,
	VL_Background1,
	VL_Main,
	VL_Overlay1,
	VL_Overlay2,
	VL_Overlay3,
	VL_Overlay4,
	VL_Screen1, // 2D
	VL_Screen2, // 2D
	VL_Screen3, // 2D
	VL_Screen4, // 2D
	_VL_COUNT
};

/* Blender like 3D viewer
* The control is inspired by the Blender 3D tool. Key 'R' resets the view to the set standard.
* The numpad keys control the camera additionally to the mouse. With right-click a context menu is shown.
*/
class PR_LIB_UI View3DWidget : public QOpenGLWidget {
	Q_OBJECT

public:
	View3DWidget(QWidget* parent = nullptr);
	virtual ~View3DWidget();

	void setCamera(const Camera& camera);
	inline const Camera& camera() const { return mCamera; }

	void setCoordinateSystem(const Frame& frame);
	inline const Frame& coordinateSystem() const { return mCoordinateSystem; }

	void addEntity(const std::shared_ptr<GraphicEntity>& entity, ViewLayer layer = VL_Main);
	void removeEntity(const std::shared_ptr<GraphicEntity>& entity);

	void clearAll();
	void clearLayer(ViewLayer layer);

	void moveEntityToLayer(const std::shared_ptr<GraphicEntity>& entity, ViewLayer layer = VL_Main);

	void enableExtraEntities(bool b = true);

	void showMainAxis(bool b);
	void setMainAxisScale(float f);

	void showScreenAxis(bool b);

	void showGrid(bool b);
	void setGridSize(float w, float h);

	void setBackgroundSolidColor(const QColor& color);
	void setBackgroundGradientColor(const QColor& start, const QColor& end);

	void enableAntialiasing(bool b = true);
	inline bool isAntialiasingEnabled() const { return mAntialiasing; }

public slots:
	void resetView();
	void frontView();
	void backView();
	void topView();
	void bottomView();
	void rightView();
	void leftView();
	void makeOrthogonal(bool b);

protected:
	void initializeGL() override;
	void resizeGL(int w, int h) override;
	void paintGL() override;

	bool event(QEvent* event) override;

	void keyPressEvent(QKeyEvent* event) override;
	void mouseMoveEvent(QMouseEvent* event) override;
	void mousePressEvent(QMouseEvent* event) override;
	void mouseReleaseEvent(QMouseEvent* event) override;
	void wheelEvent(QWheelEvent* event) override;

	void enterEvent(QEvent* event) override;
	void leaveEvent(QEvent* event) override;

private slots:
	void contextMenu(const QPoint& point);

private:
	void renderLayer(ViewLayer layer, const ShadingContext& sc);
	void renderBackground();
	void updateScreenAxis();

	void handleMoveStart(bool shift, const QPointF& localPos);
	void handleMoveUpdate(bool shift, const QPointF& localPos);
	void handleMoveEnd(bool shift, const QPointF& localPos);

	Camera mCamera;
	Frame mCoordinateSystem;

	QPointF mStartMousePos;
	QPointF mLastMousePos;

	std::array<QVector<std::shared_ptr<GraphicEntity>>, _VL_COUNT> mEntities;

	// Speed of roation on mouse drag
	float mRotationSpeed = 0.3f * PR_DEG2RAD;

	// Speed for dragging the camera around
	float mMoveSpeed = 1.0f;

	// Speed for wheel forward movement
	float mWheelSpeed = 0.002f;

	bool mAntialiasing;

	// Base Objects
	bool mExtraObjectsEnabled;
	std::shared_ptr<Axis3DEntity> mMainAxisEntity;
	std::shared_ptr<Axis3DEntity> mScreenAxisEntity;
	std::shared_ptr<GridEntity> mGridEntity;

	std::shared_ptr<ScreenSpaceEntity> mBackgroundEntity;
};
} // namespace UI
} // namespace PR