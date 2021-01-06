#pragma once

#include "ui_ImageWindow.h"
#include <QWidget>

#include <memory>

namespace PR {
namespace UI {
class ImageFile;
}
} // namespace PR

class ImageWindow : public QWidget {
	Q_OBJECT

public:
	ImageWindow(QWidget* parent = 0);
	~ImageWindow();

	void openFile(const QString& str);

private slots:
	void layerChanged();
	void exportImage();
	void updateMapper();

private:
	void updateImage(int layerID);

	Ui::ImageWindowClass ui;

	std::unique_ptr<PR::UI::ImageFile> mFile;
};