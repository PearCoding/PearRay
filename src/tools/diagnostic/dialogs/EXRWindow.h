#pragma once

#include "ui_EXRWindow.h"
#include <QWidget>

#include <memory>

namespace PR {
namespace UI {
class EXRFile;
}
} // namespace PR

class EXRWindow : public QWidget {
	Q_OBJECT

public:
	EXRWindow(QWidget* parent = 0);
	~EXRWindow();

	void openFile(const QString& str);

private slots:
	void layerChanged();
	void exportImage();
	void updateMapper();

private:
	void updateImage(int layerID);

	Ui::EXRWindowClass ui;

	std::unique_ptr<PR::UI::EXRFile> mFile;
};