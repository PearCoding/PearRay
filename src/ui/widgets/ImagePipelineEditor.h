#pragma once

#include <QWidget>

#include "PR_Config.h"

namespace PR {
namespace UI {
namespace Ui {
class ImagePipelineEditorClass;
}
class ImagePipeline;
class PR_LIB_UI ImagePipelineEditor : public QWidget {
	Q_OBJECT

public:
	ImagePipelineEditor(QWidget* parent = 0);
	~ImagePipelineEditor();

	ImagePipeline constructPipeline() const;

signals:
	void changed();

private slots:
	void gammaTypeChanged(int index);
	
private:
	Ui::ImagePipelineEditorClass* ui;
};
} // namespace UI
} // namespace PR