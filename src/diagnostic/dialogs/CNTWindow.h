#pragma once

#include <QWidget>
#include "ui_CNTWindow.h"

#include <memory>

class Container;
class GraphicObject;
class CNTWindow : public QWidget
{
	Q_OBJECT

public:
	CNTWindow(QWidget *parent = 0);
	~CNTWindow();

	void openFile(const QString& str);

private slots:
	void depthChanged(int tick);

private:
	Ui::CNTWindowClass ui;

	std::unique_ptr<Container> mContainer;
	std::shared_ptr<GraphicObject> mGraphicObject;
};