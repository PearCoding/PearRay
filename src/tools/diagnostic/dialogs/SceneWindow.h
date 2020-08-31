#pragma once

#include "ui_SceneWindow.h"
#include <QWidget>

#include <memory>

class Container;
class RayArray;
namespace PRUI {
class GraphicObject;
}
class SceneWindow : public QWidget {
	Q_OBJECT

public:
	SceneWindow(QWidget* parent = 0);
	~SceneWindow();

	void clear();
	void openCNTFile(const QString& str);
	void openRDMPFile(const QString& str);
	void openRDMPDir(const QString& str);

private:
	Ui::SceneWindowClass ui;

	struct CNTObj {
		std::shared_ptr<::Container> Container;
		std::shared_ptr<PRUI::GraphicObject> Object;
	};
	struct RDMPObj {
		std::shared_ptr<::RayArray> RayArray;
		std::shared_ptr<PRUI::GraphicObject> Object;
	};

	QVector<CNTObj> mCNTObjects;
	QVector<RDMPObj> mRDMPObjects;
};