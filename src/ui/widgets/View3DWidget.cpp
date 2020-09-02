#include "3d/OpenGLHeaders.h"

#include "3d/entities/Axis3DEntity.h"
#include "3d/entities/GraphicEntity.h"
#include "3d/entities/GridEntity.h"
#include "3d/entities/ScreenSpaceEntity.h"
#include "3d/shader/BackgroundShader.h"
#include "View3DWidget.h"

#include <QMenu>
#include <QMouseEvent>
#include <QOpenGLContext>
#include <QTimer>
#include <QWheelEvent>

#if (defined(_DEBUG) && !defined(PR_NO_OPENGL_DEBUG)) || defined(PR_FORCE_OPENGL_DEBUG)
#define PR_USE_OPENGL_DEBUG
#endif

#ifdef PR_USE_OPENGL_DEBUG
#include <QOpenGLDebugLogger>
#endif

namespace PR {
namespace UI {
constexpr ViewLayer MAIN_AXIS_LAYER	  = VL_Overlay1;
constexpr ViewLayer SCREEN_AXIS_LAYER = VL_Screen1;
constexpr ViewLayer GRID_LAYER		  = VL_Main;

constexpr ViewLayer LAYER_START	   = VL_Background4;
constexpr ViewLayer LAYER_2D_START = VL_Screen1;

constexpr float FPS			  = 30.0f;
constexpr float SCREEN_AXIS_L = 0.015f;
View3DWidget::View3DWidget(QWidget* parent)
	: QOpenGLWidget(parent)
	, mAntialiasing(false)
	, mExtraObjectsEnabled(true)
{
	setAttribute(Qt::WA_AcceptTouchEvents, true);

	QSurfaceFormat fmt = format();
	fmt.setVersion(3, 3);
	fmt.setProfile(QSurfaceFormat::CoreProfile);
#ifdef PR_USE_OPENGL_DEBUG
	fmt.setOption(QSurfaceFormat::DebugContext, true);
#endif
	fmt.setStencilBufferSize(8);
	setFormat(fmt);

	setMinimumSize(320, 240);
	setFocusPolicy(Qt::ClickFocus);

	mBackgroundEntity = std::make_shared<ScreenSpaceEntity>();

	// Extra objects
	mMainAxisEntity	  = std::make_shared<Axis3DEntity>();
	mScreenAxisEntity = std::make_shared<Axis3DEntity>(SCREEN_AXIS_L);
	mGridEntity		  = std::make_shared<GridEntity>();

	addEntity(mMainAxisEntity, MAIN_AXIS_LAYER);
	addEntity(mScreenAxisEntity, SCREEN_AXIS_LAYER);
	addEntity(mGridEntity, GRID_LAYER);

	updateScreenAxis();

	setContextMenuPolicy(Qt::CustomContextMenu);
	connect(this, &View3DWidget::customContextMenuRequested, this, &View3DWidget::contextMenu);

	// Try to update with a fixed FPS
	QTimer* timer = new QTimer(this);
	connect(timer, &QTimer::timeout, [this]() { this->update(); });
	timer->start(1000 / FPS);

	setCamera(Camera()); // Update extra entity transform
}

View3DWidget::~View3DWidget()
{
	makeCurrent(); // Make sure every call inside entities/shaders has a proper context
	for (int i = 0; i < _VL_COUNT; ++i)
		mEntities[i].clear();
}

void View3DWidget::setBackgroundSolidColor(const QColor& color)
{
	auto shader = std::dynamic_pointer_cast<BackgroundShader>(mBackgroundEntity->shader());
	shader->setColorStart(Vector4f(color.redF(), color.greenF(), color.blueF(), color.alphaF()));
	shader->setColorEnd(Vector4f(color.redF(), color.greenF(), color.blueF(), color.alphaF()));
}

void View3DWidget::setBackgroundGradientColor(const QColor& start, const QColor& end)
{
	auto shader = std::dynamic_pointer_cast<BackgroundShader>(mBackgroundEntity->shader());
	shader->setColorStart(Vector4f(start.redF(), start.greenF(), start.blueF(), start.alphaF()));
	shader->setColorEnd(Vector4f(end.redF(), end.greenF(), end.blueF(), end.alphaF()));
}

void View3DWidget::contextMenu(const QPoint& point)
{
	QMenu contextMenu(tr("Context menu"), this);

	if (mExtraObjectsEnabled) {
		QAction* action1 = new QAction("Show Main Axis", &contextMenu);
		action1->setCheckable(true);
		action1->setChecked(mMainAxisEntity->isVisible());
		connect(action1, &QAction::triggered, [&](bool checked) {mMainAxisEntity->show(checked); update(); });
		contextMenu.addAction(action1);

		QAction* action2 = new QAction("Show Screen Axis", &contextMenu);
		action2->setCheckable(true);
		action2->setChecked(mScreenAxisEntity->isVisible());
		connect(action2, &QAction::triggered, [&](bool checked) {mScreenAxisEntity->show(checked); update(); });
		contextMenu.addAction(action2);

		QAction* action3 = new QAction("Show Grid", &contextMenu);
		action3->setCheckable(true);
		action3->setChecked(mGridEntity->isVisible());
		connect(action3, &QAction::triggered, [&](bool checked) {mGridEntity->show(checked); update(); });
		contextMenu.addAction(action3);

		contextMenu.addSeparator();
	}

	QAction* actionOrth = new QAction("Orthogonal", &contextMenu);
	actionOrth->setCheckable(true);
	actionOrth->setChecked(mCamera.isOrthogonal());
	connect(actionOrth, &QAction::triggered, this, &View3DWidget::makeOrthogonal);
	contextMenu.addAction(actionOrth);

	contextMenu.addSeparator();

	QAction* action = new QAction("Reset View", &contextMenu);
	connect(action, &QAction::triggered, this, &View3DWidget::resetView);
	contextMenu.addAction(action);

	action = new QAction("Front View", &contextMenu);
	connect(action, &QAction::triggered, this, &View3DWidget::frontView);
	contextMenu.addAction(action);

	action = new QAction("Back View", &contextMenu);
	connect(action, &QAction::triggered, this, &View3DWidget::backView);
	contextMenu.addAction(action);

	action = new QAction("Top View", &contextMenu);
	connect(action, &QAction::triggered, this, &View3DWidget::topView);
	contextMenu.addAction(action);

	action = new QAction("Bottom View", &contextMenu);
	connect(action, &QAction::triggered, this, &View3DWidget::bottomView);
	contextMenu.addAction(action);

	action = new QAction("Left View", &contextMenu);
	connect(action, &QAction::triggered, this, &View3DWidget::leftView);
	contextMenu.addAction(action);

	action = new QAction("Right View", &contextMenu);
	connect(action, &QAction::triggered, this, &View3DWidget::rightView);
	contextMenu.addAction(action);

	contextMenu.exec(mapToGlobal(point));
}

void View3DWidget::setCamera(const Camera& camera)
{
	mCamera = camera;
}

void View3DWidget::setCoordinateSystem(const Frame& frame)
{
	mCoordinateSystem = frame;

	mMainAxisEntity->setRotation(Quaternionf(mCoordinateSystem.matrix()));
	mGridEntity->setRotation(Quaternionf(mCoordinateSystem.matrix()));
}

void View3DWidget::addEntity(const std::shared_ptr<GraphicEntity>& entity, ViewLayer layer)
{
	mEntities[layer].append(entity);
}

void View3DWidget::removeEntity(const std::shared_ptr<GraphicEntity>& entity)
{
	for (int i = 0; i < _VL_COUNT; ++i)
		mEntities[i].removeOne(entity);
}

void View3DWidget::clearAll()
{
	for (int i = 0; i < _VL_COUNT; ++i)
		clearLayer((ViewLayer)i);
}

void View3DWidget::clearLayer(ViewLayer layer)
{
	mEntities[layer].clear();

	// Make sure the extra entities are back
	if (layer == GRID_LAYER)
		addEntity(mGridEntity, GRID_LAYER);
	if (layer == MAIN_AXIS_LAYER)
		addEntity(mMainAxisEntity, MAIN_AXIS_LAYER);
	if (layer == SCREEN_AXIS_LAYER)
		addEntity(mScreenAxisEntity, SCREEN_AXIS_LAYER);
}

void View3DWidget::moveEntityToLayer(const std::shared_ptr<GraphicEntity>& entity, ViewLayer layer)
{
	removeEntity(entity);
	mEntities[layer].append(entity); // Last drawn is top
}

constexpr int MULTISAMPLE_COUNT = 8;
void View3DWidget::enableAntialiasing(bool b)
{
	if (b != mAntialiasing) {
		QSurfaceFormat fmt = format();
		fmt.setSamples(b ? MULTISAMPLE_COUNT : 1);
		setFormat(fmt);
	}
	mAntialiasing = b;
}

static void* _gl_getProcAddr(const char* name)
{
	return (void*)QOpenGLContext::currentContext()->getProcAddress(name);
}

void View3DWidget::initializeGL()
{
	init_opengl_functions(_gl_getProcAddr);

#ifdef PR_USE_OPENGL_DEBUG
	QOpenGLDebugLogger* logger = new QOpenGLDebugLogger(this);
	logger->initialize();
	connect(logger, &QOpenGLDebugLogger::messageLogged,
			[](const QOpenGLDebugMessage& debugMessage) {
				switch (debugMessage.severity()) {
				default:
				case QOpenGLDebugMessage::HighSeverity:
					qCritical() << debugMessage;
					break;
				case QOpenGLDebugMessage::MediumSeverity:
					qWarning() << debugMessage;
					break;
				case QOpenGLDebugMessage::LowSeverity:
					qDebug() << debugMessage;
					break;
				case QOpenGLDebugMessage::NotificationSeverity:
					qInfo() << debugMessage;
					break;
				}
			});
	logger->startLogging();
#endif

	GL_CHECK(glEnable(GL_CULL_FACE));
	GL_CHECK(glEnable(GL_BLEND));
	GL_CHECK(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
	GL_CHECK(glClearColor(0, 0, 0, 0));
}

void View3DWidget::resizeGL(int w, int h)
{
	mCamera.setScreenSize(w, h);
	updateScreenAxis();
}

void View3DWidget::paintGL()
{
	GL_CHECK(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
#ifndef PR_OPENGL_ES
	if (mAntialiasing) {
		GL_CHECK(glEnable(GL_MULTISAMPLE));
		GL_CHECK(glEnable(GL_LINE_SMOOTH));
	} else {
		GL_CHECK(glDisable(GL_MULTISAMPLE));
		GL_CHECK(glDisable(GL_LINE_SMOOTH));
	}
#endif
	renderBackground();
	GL_CHECK(glEnable(GL_DEPTH_TEST));

	const auto SC = ShadingContext(mCamera.constructViewMatrix(), mCamera.constructCameraMatrix());

	for (int layer = LAYER_START; layer < LAYER_2D_START; ++layer) {
		if (mEntities[layer].empty())
			continue;
		renderLayer((ViewLayer)layer, SC);
	}

	GL_CHECK(glDisable(GL_DEPTH_TEST));
	updateScreenAxis();
	const auto S = ShadingContext(Matrix4f::Identity(), mCamera.constructScreenMatrix());
	for (int layer = LAYER_2D_START; layer < _VL_COUNT; ++layer) {
		if (mEntities[layer].empty())
			continue;
		renderLayer((ViewLayer)layer, S);
	}
}

void View3DWidget::renderLayer(ViewLayer layer, const ShadingContext& sc)
{
	GL_CHECK(glClear(GL_DEPTH_BUFFER_BIT));

	std::vector<std::shared_ptr<GraphicEntity>> transEnts;
	// First render all opaque entities
	for (const auto& ptr : mEntities[layer]) {
		if (ptr->isVisible()) {
			bool hasTransparency = ptr->shader() && ptr->shader()->hasTransparency();
			if (hasTransparency) {
				transEnts.push_back(ptr);
				continue;
			}

			ptr->render(sc);
		}
	}

	if (!transEnts.empty()) {
		// Sort transEntries
		std::sort(transEnts.begin(), transEnts.end(),
				  [&](const std::shared_ptr<GraphicEntity>& a, const std::shared_ptr<GraphicEntity>& b) {
					  float a_d = (a->position() - mCamera.position()).squaredNorm();
					  float b_d = (b->position() - mCamera.position()).squaredNorm();
					  return a_d > b_d;
				  });

		// Render transparent entities
		for (const auto& ptr : transEnts) {
			bool isTwoSided = ptr->isTwoSided();
			ptr->setTwoSided(true);
			ptr->render(sc);
			ptr->setTwoSided(isTwoSided);
		}
	}
}

void View3DWidget::renderBackground()
{
	GL_CHECK(glDisable(GL_DEPTH_TEST));
	mBackgroundEntity->render(ShadingContext());
}

// Blender like numpad control
void View3DWidget::keyPressEvent(QKeyEvent* event)
{
	constexpr float ROT_DELTA  = 10.0f * PR::PR_DEG2RAD;
	constexpr float PAD_DELTA  = 0.25f;
	constexpr float ZOOM_DELTA = 0.25f;

	float pivPosDist = (mCamera.pivot() - mCamera.position()).norm();

	switch (event->key()) {
	case Qt::Key_1:
		if (event->modifiers() & Qt::KeypadModifier) {
			if (event->modifiers() & Qt::ControlModifier)
				backView();
			else
				frontView();
		}
		break;
	case Qt::Key_2:
		if (event->modifiers() & Qt::KeypadModifier) {
			if (event->modifiers() & Qt::ControlModifier)
				mCamera.pan(0, -pivPosDist * PAD_DELTA);
			else
				mCamera.rotateView(0, ROT_DELTA);
			update();
		}
		break;
	case Qt::Key_3:
		if (event->modifiers() & Qt::KeypadModifier) {
			if (event->modifiers() & Qt::ControlModifier)
				leftView();
			else
				rightView();
		}
		break;
	case Qt::Key_4:
		if (event->modifiers() & Qt::KeypadModifier) {
			if (event->modifiers() & Qt::ControlModifier)
				mCamera.pan(pivPosDist * PAD_DELTA, 0);
			else
				mCamera.rotateView(-ROT_DELTA, 0);
			update();
		}
		break;
	case Qt::Key_5:
		if (event->modifiers() & Qt::KeypadModifier)
			makeOrthogonal(!mCamera.isOrthogonal());
		break;
	case Qt::Key_6:
		if (event->modifiers() & Qt::KeypadModifier) {
			if (event->modifiers() & Qt::ControlModifier)
				mCamera.pan(-pivPosDist * PAD_DELTA, 0);
			else
				mCamera.rotateView(ROT_DELTA, 0);
			update();
		}
		break;
	case Qt::Key_7:
		if (event->modifiers() & Qt::KeypadModifier) {
			if (event->modifiers() & Qt::ControlModifier)
				bottomView();
			else
				topView();
		}
		break;
	case Qt::Key_8:
		if (event->modifiers() & Qt::KeypadModifier) {
			if (event->modifiers() & Qt::ControlModifier)
				mCamera.pan(0, pivPosDist * PAD_DELTA);
			else
				mCamera.rotateView(0, -ROT_DELTA);
			update();
		}
		break;
	case Qt::Key_9:
		if (event->modifiers() & Qt::KeypadModifier) {
			mCamera.rotate(-180.0f, 0);
			update();
		}
		break;
	case Qt::Key_Plus:
		if (event->modifiers() & Qt::KeypadModifier) {
			mCamera.zoom(ZOOM_DELTA);
			update();
		}
		break;
	case Qt::Key_Minus:
		if (event->modifiers() & Qt::KeypadModifier) {
			mCamera.zoom(-ZOOM_DELTA);
			update();
		}
		break;
	case Qt::Key_R:
		resetView();
		break;
	}
}

void View3DWidget::updateScreenAxis()
{
	constexpr float PAD	 = 0.002f;
	Matrix3f orientation = mCamera.constructViewMatrix().block<3, 3>(0, 0) * mCoordinateSystem.matrix();
	mScreenAxisEntity->setTransform(
		Vector3f(SCREEN_AXIS_L + PAD, SCREEN_AXIS_L + PAD, 0),
		Quaternionf(orientation));
}

bool View3DWidget::event(QEvent* event)
{
	switch (event->type()) {
	case QEvent::TouchBegin: {
		QTouchEvent* tevent = (QTouchEvent*)event;
		if (tevent->touchPoints().size() > 0) {
			// TODO: Special case size==2?
			auto point = tevent->touchPoints().at(0);
			handleMoveStart(tevent->modifiers() & Qt::ShiftModifier, point.pos());
			tevent->accept();
			return true;
		} else {
			tevent->ignore();
			return true;
		}
	}
	case QEvent::TouchEnd: {
		QTouchEvent* tevent = (QTouchEvent*)event;
		if (tevent->touchPoints().size() > 0) {
			// TODO: Special case size==2?
			auto point = tevent->touchPoints().at(0);
			handleMoveEnd(tevent->modifiers() & Qt::ShiftModifier, point.pos());
			tevent->accept();
			return true;
		} else {
			tevent->ignore();
			return true;
		}
	}
	case QEvent::TouchCancel:
		return false; // Ignore
	case QEvent::TouchUpdate: {
		QTouchEvent* tevent = (QTouchEvent*)event;
		if (tevent->touchPoints().size() > 0) {
			// TODO: Special case size==2?
			auto point = tevent->touchPoints().at(0);
			handleMoveUpdate(tevent->modifiers() & Qt::ShiftModifier, point.pos());
			tevent->accept();
			return true;
		} else {
			tevent->ignore();
			return true;
		}
	}
	default:
		break;
	}
	return QWidget::event(event);
}

void View3DWidget::mouseMoveEvent(QMouseEvent* event)
{
	if (event->buttons() & Qt::LeftButton)
		handleMoveUpdate(event->modifiers() & Qt::ShiftModifier, event->localPos());

	if (event->buttons() & Qt::MidButton)
		handleMoveUpdate(event->modifiers() & Qt::ShiftModifier, event->localPos());

	mLastMousePos = event->localPos();
}

void View3DWidget::mousePressEvent(QMouseEvent* event)
{
	if (event->button() == Qt::LeftButton)
		handleMoveStart(event->modifiers() & Qt::ShiftModifier, event->localPos());
	else if (event->button() == Qt::MidButton)
		handleMoveStart(event->modifiers() & Qt::ShiftModifier, event->localPos());
}

void View3DWidget::mouseReleaseEvent(QMouseEvent* event)
{
	if (event->button() == Qt::LeftButton)
		handleMoveEnd(event->modifiers() & Qt::ShiftModifier, event->localPos());
	else if (event->button() == Qt::MidButton)
		handleMoveEnd(event->modifiers() & Qt::ShiftModifier, event->localPos());
}

void View3DWidget::handleMoveStart(bool shift, const QPointF& localPos)
{
	Q_UNUSED(shift);
	mLastMousePos  = localPos;
	mStartMousePos = mLastMousePos;
}

void View3DWidget::handleMoveUpdate(bool shift, const QPointF& localPos)
{
	if (shift) {
		// TODO: There is an issue with orthogonal projection here!
		QPointF delta = localPos - mLastMousePos;
		if (delta.manhattanLength() > 0.001f) {
			float length	= (mCamera.pivot() - mCamera.position()).norm();
			Vector3f deltaV = length * mMoveSpeed * mCamera.projectLine(Vector2f(delta.x(), delta.y()));
			mCamera.translateWithPivot(-deltaV);

			update();
		}
	} else {
		auto feedbackCurve = [](float dist, float pivot = 20.0f) {
			return std::min<float>(1.0f, std::abs(dist) / pivot);
		};

		float alpha = -mRotationSpeed * feedbackCurve(localPos.x() - mStartMousePos.x()) * (localPos.x() - mLastMousePos.x());
		float beta	= mRotationSpeed * feedbackCurve(localPos.y() - mStartMousePos.y()) * (localPos.y() - mLastMousePos.y());
		mCamera.rotate(alpha, beta);

		update();
	}
}

void View3DWidget::handleMoveEnd(bool shift, const QPointF& localPos)
{
	Q_UNUSED(shift);
	Q_UNUSED(localPos);
}

void View3DWidget::wheelEvent(QWheelEvent* event)
{
	float movement = event->delta() * mWheelSpeed;
	mCamera.zoom(movement);
	update();
}

void View3DWidget::enterEvent(QEvent* event)
{
	Q_UNUSED(event);
	grabKeyboard();
}

void View3DWidget::leaveEvent(QEvent* event)
{
	Q_UNUSED(event);
	releaseKeyboard();
}

void View3DWidget::enableExtraEntities(bool b)
{
	mExtraObjectsEnabled = b;
	mMainAxisEntity->show(b);
	mScreenAxisEntity->show(b);
	mGridEntity->show(b);
}

void View3DWidget::showMainAxis(bool b)
{
	mMainAxisEntity->show(b);
}

void View3DWidget::setMainAxisScale(float f)
{
	mMainAxisEntity->setEdgeLength(f);
}

void View3DWidget::showScreenAxis(bool b)
{
	mScreenAxisEntity->show(b);
}

void View3DWidget::showGrid(bool b)
{
	mGridEntity->show(b);
}

void View3DWidget::setGridSize(float w, float h)
{
	mGridEntity->setGridSize(w, h);
}

void View3DWidget::resetView()
{
	mCamera.setOrientation(mCoordinateSystem);
	mCamera.setPivot(Vector3f(0, 0, 0));
	mCamera.setPosition(-1.0f * mCoordinateSystem.Front);
	update();
}

void View3DWidget::frontView()
{
	mCamera.makeFrontView(mCoordinateSystem);
	update();
}

void View3DWidget::backView()
{
	mCamera.makeBackView(mCoordinateSystem);
	update();
}

void View3DWidget::topView()
{
	mCamera.makeTopView(mCoordinateSystem);
	update();
}

void View3DWidget::bottomView()
{
	mCamera.makeBottomView(mCoordinateSystem);
	update();
}

void View3DWidget::rightView()
{
	mCamera.makeRightView(mCoordinateSystem);
	update();
}

void View3DWidget::leftView()
{
	mCamera.makeLeftView(mCoordinateSystem);
	update();
}

void View3DWidget::makeOrthogonal(bool b)
{
	mCamera.makeOrthogonal(b);
	update();
}
} // namespace UI
} // namespace PR