#include "ToneMapperEditor.h"
#include "mapper/ToneMapper.h"
#include "ui_ToneMapperEditor.h"

namespace PRUI {
ToneMapperEditor::ToneMapperEditor(QWidget* parent)
	: QWidget(parent), ui(new Ui::ToneMapperEditorClass)
{
	ui->setupUi(this);

	connect(ui->rangeSlider, SIGNAL(leftValueChanged(float)), this, SLOT(valueChanged()));
	connect(ui->rangeSlider, SIGNAL(rightValueChanged(float)), this, SLOT(valueChanged()));

	connect(ui->absoluteCB, SIGNAL(stateChanged(int)), this, SIGNAL(changed()));

	connect(ui->normalButton, SIGNAL(clicked()), this, SLOT(normalButtonPushed()));
	connect(ui->fitButton, SIGNAL(clicked()), this, SLOT(fitButtonPushed()));

	connect(ui->formatToCB, SIGNAL(currentIndexChanged(int)), this, SIGNAL(formatChanged()));
	connect(ui->formatFromCB, SIGNAL(currentIndexChanged(int)), this, SIGNAL(formatChanged()));

	connect(ui->gammaCB, SIGNAL(stateChanged(int)), this, SIGNAL(formatChanged()));
	connect(ui->gammaSB, SIGNAL(valueChanged(double)), this, SIGNAL(formatChanged()));
}

ToneMapperEditor::~ToneMapperEditor()
{
	delete ui;
}

void ToneMapperEditor::setMinMax(float min, float max)
{
	ui->rangeSlider->setMinValue(min);
	ui->rangeSlider->setMaxValue(max);
	valueChanged();
}

void ToneMapperEditor::setToNormal()
{
	ui->rangeSlider->setLeftValue(0.0f);
	ui->rangeSlider->setRightValue(1.0f);
	ui->rangeSlider->repaint();
}

ToneMapper ToneMapperEditor::constructMapper() const
{
	ToneMapper mapper;

	mapper.setMin(ui->rangeSlider->leftValue());
	mapper.setMax(ui->rangeSlider->rightValue());
	mapper.enableAbsolute(ui->absoluteCB->isChecked());
	mapper.setTripletFormat((ColorFormat)ui->formatFromCB->currentIndex(),
							(ColorFormat)ui->formatToCB->currentIndex());

	if (ui->gammaCB->isChecked())
		mapper.setGamma(ui->gammaSB->value());
	else
		mapper.setGamma(-1);

	return mapper;
}

void ToneMapperEditor::normalButtonPushed()
{
	setToNormal();
}

void ToneMapperEditor::fitButtonPushed()
{
	ui->rangeSlider->setLeftValue(-std::numeric_limits<float>::infinity());
	ui->rangeSlider->setRightValue(std::numeric_limits<float>::infinity());
	ui->rangeSlider->repaint();
}

void ToneMapperEditor::valueChanged()
{
	ui->rangeLabel->setText(QString("{%1, %2} -> {%3, %4}")
							   .arg(ui->rangeSlider->minValue())
							   .arg(ui->rangeSlider->maxValue())
							   .arg(ui->rangeSlider->leftValue())
							   .arg(ui->rangeSlider->rightValue()));

	emit changed();
}
}