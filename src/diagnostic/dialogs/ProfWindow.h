#pragma once

#include "ui_ProfWindow.h"
#include <QWidget>
#include <QSignalMapper>

#include <memory>

class ProfFile;
class ProfTreeModel;
class ProfWindow : public QWidget {
	Q_OBJECT

public:
	ProfWindow(QWidget* parent = 0);
	~ProfWindow();

	void openFile(const QString& str);

private slots:
	void exportImage();

	void onItemContextMenu(const QPoint& p);
	void onItemShowPlot(QObject* obj);

private:
	void setupModel();
	Ui::ProfWindowClass ui;

	std::shared_ptr<ProfFile> mContext;
	std::shared_ptr<ProfTreeModel> mTreeModel;

	QSignalMapper mSignalMapper;
};