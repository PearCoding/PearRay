#include "ImagePipelineEditor.h"
#include "mapper/ImagePipeline.h"
#include "ui_ImagePipelineEditor.h"

namespace PR {
namespace UI {
ImagePipelineEditor::ImagePipelineEditor(QWidget* parent)
	: QWidget(parent)
	, ui(new Ui::ImagePipelineEditorClass)
{
	ui->setupUi(this);

	connect(ui->exposureSB, SIGNAL(valueChanged(double)), this, SIGNAL(changed()));
	connect(ui->offsetSB, SIGNAL(valueChanged(double)), this, SIGNAL(changed()));

	connect(ui->rangeCB, SIGNAL(stateChanged(int)), this, SIGNAL(changed()));
	connect(ui->rangeSlider, SIGNAL(leftValueChanged(float)), this, SIGNAL(changed()));
	connect(ui->rangeSlider, SIGNAL(rightValueChanged(float)), this, SIGNAL(changed()));

	connect(ui->toneMapperCB, SIGNAL(currentIndexChanged(int)), this, SIGNAL(changed()));
	connect(ui->formatToCB, SIGNAL(currentIndexChanged(int)), this, SIGNAL(changed()));
	connect(ui->formatFromCB, SIGNAL(currentIndexChanged(int)), this, SIGNAL(changed()));

	connect(ui->gammaCB, SIGNAL(stateChanged(int)), this, SIGNAL(changed()));
	connect(ui->invertCB, SIGNAL(stateChanged(int)), this, SIGNAL(changed()));
	connect(ui->gammaSB, SIGNAL(valueChanged(double)), this, SIGNAL(changed()));
}

ImagePipelineEditor::~ImagePipelineEditor()
{
	delete ui;
}

ImagePipeline ImagePipelineEditor::constructPipeline() const
{
	ImagePipeline mapper;

	mapper.setExposure(ui->exposureSB->value());
	mapper.setOffset(ui->offsetSB->value());

	if (ui->rangeCB->isChecked()) {
		mapper.setMinRange(ui->rangeSlider->leftValue());
		mapper.setMaxRange(ui->rangeSlider->rightValue());
	}

	mapper.setToneMapping((ToneMappingMode)ui->toneMapperCB->currentIndex());

	mapper.setTripletFormat((ColorFormat)ui->formatFromCB->currentIndex(),
							(ColorFormat)ui->formatToCB->currentIndex());

	if (ui->gammaCB->isChecked()) {
		if (ui->invertCB->isChecked())
			mapper.setGamma(1.0f / ui->gammaSB->value());
		else
			mapper.setGamma(ui->gammaSB->value());
	} else {
		mapper.setGamma(-1);
	}

	return mapper;
}
} // namespace UI
} // namespace PR