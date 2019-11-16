#pragma once

#include "ui_ProfWindow.h"
#include <QWidget>

#include <memory>

class ProfFile;
class ProfTreeModel;
class ProfWindow : public QWidget {
	Q_OBJECT

public:
	ProfWindow(QWidget* parent = 0);
	~ProfWindow();

	void openFile(const QString& str);

private:
	void setupModel();
	void setupThreadList();
	void setupPlot();
	Ui::ProfWindowClass ui;

	std::shared_ptr<ProfFile> mContext;
	std::shared_ptr<ProfTreeModel> mTreeModel;
};