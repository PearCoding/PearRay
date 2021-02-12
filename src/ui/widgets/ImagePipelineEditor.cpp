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

	// Set defaults
	ui->toneMapperCB->setCurrentIndex((int)ToneMappingMode::ACES);
	ui->formatToCB->setCurrentIndex((int)ColorFormat::SRGB);
	ui->formatFromCB->setCurrentIndex((int)ColorFormat::XYZ /* Renderer default */);
	ui->gammaType->setCurrentIndex((int)GammaType::SRGB);
	ui->gammaSB->setValue(2.20f);
	ui->invertCB->setChecked(true);

	gammaTypeChanged((int)GammaType::SRGB);

	connect(ui->exposureSB, SIGNAL(valueChanged(double)), this, SIGNAL(changed()));
	connect(ui->offsetSB, SIGNAL(valueChanged(double)), this, SIGNAL(changed()));

	connect(ui->rangeCB, SIGNAL(stateChanged(int)), this, SIGNAL(changed()));
	connect(ui->rangeSlider, SIGNAL(leftValueChanged(float)), this, SIGNAL(changed()));
	connect(ui->rangeSlider, SIGNAL(rightValueChanged(float)), this, SIGNAL(changed()));

	connect(ui->toneMapperCB, SIGNAL(currentIndexChanged(int)), this, SIGNAL(changed()));
	connect(ui->formatToCB, SIGNAL(currentIndexChanged(int)), this, SIGNAL(changed()));
	connect(ui->formatFromCB, SIGNAL(currentIndexChanged(int)), this, SIGNAL(changed()));

	connect(ui->gammaType, SIGNAL(currentIndexChanged(int)), this, SIGNAL(changed()));
	connect(ui->invertCB, SIGNAL(stateChanged(int)), this, SIGNAL(changed()));
	connect(ui->gammaSB, SIGNAL(valueChanged(double)), this, SIGNAL(changed()));

	connect(ui->gammaType, SIGNAL(currentIndexChanged(int)), this, SLOT(gammaTypeChanged(int)));
}

ImagePipelineEditor::~ImagePipelineEditor()
{
	delete ui;
}

void ImagePipelineEditor::gammaTypeChanged(int index)
{
	const bool enable = (index == (int)GammaType::Custom);
	ui->invertCB->setEnabled(enable);
	ui->invertCB->setVisible(enable);
	ui->gammaSB->setEnabled(enable);
	ui->gammaSB->setVisible(enable);
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

	if (ui->invertCB->isChecked())
		mapper.setGamma(1.0f / ui->gammaSB->value());
	else
		mapper.setGamma(ui->gammaSB->value());

	mapper.setGammaType((GammaType)ui->gammaType->currentIndex());

	return mapper;
}
} // namespace UI
} // namespace PR