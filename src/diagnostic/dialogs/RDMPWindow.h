#pragma once

#include <QWidget>
#include "ui_RDMPWindow.h"

#include <memory>

class GraphicObject;
class RayArray;
class RDMPWindow : public QWidget
{
	Q_OBJECT

public:
	RDMPWindow(QWidget *parent = 0);
	~RDMPWindow();

	void openFile(const QString& str);
	void openDir(const QString& str);

private:
	Ui::RDMPWindowClass ui;

	std::unique_ptr<RayArray> mRays;
	std::shared_ptr<GraphicObject> mGraphicObject;
};