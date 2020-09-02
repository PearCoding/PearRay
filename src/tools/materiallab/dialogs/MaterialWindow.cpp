#include "MaterialWindow.h"
#include "SceneLoadContext.h"
#include "material/IMaterial.h"
#include "material/IMaterialPlugin.h"
#include "renderer/RenderTileSession.h"

#include "properties/BoolProperty.h"
#include "properties/DoubleProperty.h"
#include "properties/IntProperty.h"
#include "properties/PropertyContainer.h"
#include "properties/TextProperty.h"

#include "3d/entities/HemiFunctionEntity.h"
#include "3d/entities/InstanceEntity.h"
#include "3d/entities/SphereEntity.h"
#include "3d/shader/ColorShader.h"
#include "3d/shader/WireframeShader.h"

#include <QInputDialog>
#include <QMessageBox>

using namespace PR;

constexpr float SUN_R = 0.1f;
constexpr float SUN_D = 4.0f;
constexpr int ANIM_MS = 200;
MaterialWindow::MaterialWindow(const QString& typeName, PR::IMaterialPlugin* factory, QWidget* parent)
	: QWidget(parent)
	, mTypeName(typeName)
	, mFactory(factory)
	, mProperties(new PR::UI::PropertyContainer())
{
	PR_ASSERT(mFactory, "Expected valid factory");

	ui.setupUi(this);

	connect(ui.addPropertyButton, &QPushButton::clicked, this, &MaterialWindow::addProperty);
	connect(ui.createButton, &QPushButton::clicked, this, &MaterialWindow::createMaterial);
	connect(ui.sunThetaSlider, &QSlider::valueChanged, this, &MaterialWindow::controlChanged);
	connect(ui.sunPhiSlider, &QSlider::valueChanged, this, &MaterialWindow::controlChanged);
	connect(ui.wavelengthSlider, &QSlider::valueChanged, this, &MaterialWindow::controlChanged);
	connect(ui.swapCB, &QCheckBox::stateChanged, this, &MaterialWindow::controlChanged);
	connect(ui.ndotlCB, &QCheckBox::stateChanged, this, &MaterialWindow::controlChanged);
	connect(ui.normCB, &QCheckBox::stateChanged, this, &MaterialWindow::controlChanged);

	connect(ui.wireframeCB, &QCheckBox::stateChanged, this, &MaterialWindow::displayChanged);
	connect(ui.brdfCB, &QCheckBox::stateChanged, this, &MaterialWindow::displayChanged);
	connect(ui.btdfCB, &QCheckBox::stateChanged, this, &MaterialWindow::displayChanged);

	mAnimationTimer.setInterval(ANIM_MS);
	connect(&mAnimationTimer, &QTimer::timeout, this, &MaterialWindow::animationHandler);

	connect(ui.animButton, &QPushButton::clicked, [this]() {
		if (ui.animButton->isChecked())
			mAnimationTimer.start();
		else
			mAnimationTimer.stop();
	});

	setWindowTitle(mTypeName);

	ui.propertyView->setPropertyContainer(mProperties);

	ui.sceneView->enableAntialiasing();
	ui.sceneView->enableExtraEntities();

	// Setup light source
	mLightSource = std::make_shared<PR::UI::SphereEntity>(SUN_R);
	mLightSource->setShader(std::make_shared<PR::UI::ColorShader>(Vector4f(0.8f, 0.9f, 0, 1)));
	ui.sceneView->addEntity(mLightSource, PR::UI::VL_Overlay1);
	controlChanged();

	populateInfo();
}

MaterialWindow::~MaterialWindow()
{
	delete mProperties;
}

void MaterialWindow::addProperty()
{
	static const QStringList propertytypes({ "String", "Bool", "Integer", "Number" });

	bool ok;
	QString item = QInputDialog::getItem(this, tr("Select property type"), tr("Type:"), propertytypes, 0, false, &ok);

	if (ok && !item.isEmpty()) {
		if (item == "String")
			ui.propertyView->addProperty(new PR::UI::TextProperty());
		else if (item == "Bool")
			ui.propertyView->addProperty(new PR::UI::BoolProperty());
		else if (item == "Integer")
			ui.propertyView->addProperty(new PR::UI::IntProperty());
		else
			ui.propertyView->addProperty(new PR::UI::DoubleProperty());
	}
}

void MaterialWindow::createMaterial()
{
	PR::SceneLoadContext ctx;
	for (auto prop : mProperties->topProperties()) {
		if (prop->propertyName().isEmpty())
			continue;

		if (auto* p = dynamic_cast<PR::UI::TextProperty*>(prop))
			ctx.parameters().addParameter(p->propertyName().toStdString(), PR::Parameter::fromString(p->text().toStdString()));
		else if (auto* p = dynamic_cast<PR::UI::BoolProperty*>(prop))
			ctx.parameters().addParameter(p->propertyName().toStdString(), PR::Parameter::fromBool(p->value()));
		else if (auto* p = dynamic_cast<PR::UI::IntProperty*>(prop))
			ctx.parameters().addParameter(p->propertyName().toStdString(), PR::Parameter::fromInt(p->value()));
		else if (auto* p = dynamic_cast<PR::UI::DoubleProperty*>(prop))
			ctx.parameters().addParameter(p->propertyName().toStdString(), PR::Parameter::fromNumber(p->value()));
	}

	try {
		mMaterial = mFactory->create(0, mTypeName.toLocal8Bit().data(), ctx);
	} catch (const std::exception& e) {
		QMessageBox::critical(this, tr("Could not create material"), tr("Could not create material of type %1:\n%2").arg(mTypeName).arg(e.what()));
		mMaterial = nullptr;
		return;
	}

	if (!mMaterial) {
		QMessageBox::critical(this, tr("Could not create material"),
							  tr("Could not create material of type %1:\nFactory rejected creation. Creation for query environments is probably not supported.").arg(mTypeName));
		return;
	}

	const std::string info = mMaterial->dumpInformation();
	ui.dumpTextView->setPlainText(QString::fromStdString(info));

	buildGraphicObjects();
}

void MaterialWindow::populateInfo()
{
	QStringList names;
	for (auto name : mFactory->getNames())
		names.append(QString::fromStdString(name));
	ui.namesView->addItems(names);
}

void MaterialWindow::controlChanged()
{
	float theta		 = ui.sunThetaSlider->value() * PR::PR_DEG2RAD;
	float phi		 = ui.sunPhiSlider->value() * PR::PR_DEG2RAD;
	const Vector3f L = PR::Spherical::cartesian(theta, phi);
	mLightSource->setTranslation(SUN_D * L);

	const Vector3f mL = generateL();
	ui.lDisplay->setText(QString("[%1, %2, %3]").arg(mL.x(), 4, 'f', 4).arg(mL.y(), 4, 'f', 4).arg(mL.z(), 4, 'f', 4));

	if (mMaterial)
		buildGraphicObjects();
}

void MaterialWindow::displayChanged()
{
	if (!mBRDFObject)
		return;

	mBRDFObject->show(ui.brdfCB->isChecked());
	mBRDFObjectOutline->show(ui.wireframeCB->isChecked() && ui.brdfCB->isChecked());
	mBTDFObject->show(ui.btdfCB->isChecked());
	mBTDFObjectOutline->show(ui.wireframeCB->isChecked() && ui.btdfCB->isChecked());
}

void MaterialWindow::animationHandler()
{
	int theta = ui.sunThetaSlider->value();
	int phi	  = ui.sunPhiSlider->value();

	ui.sunThetaSlider->setValue((theta + 1) % 180);
	ui.sunPhiSlider->setValue((phi + 3) % 360);
}

void MaterialWindow::buildGraphicObjects()
{
	if (!mBRDFObject) {
		mBRDFObject = std::make_shared<PR::UI::HemiFunctionEntity>([this](const Vector3f& D) { return evalBSDF(D); });
		mBRDFObject->enableNormalization(ui.normCB->isChecked());
		ui.sceneView->addEntity(mBRDFObject);

		// Wireframe
		mBRDFObjectOutline = std::make_shared<PR::UI::InstanceEntity>(mBRDFObject);
		mBRDFObjectOutline->setShader(std::make_shared<PR::UI::WireframeShader>(Vector4f(0.2f, 0.2f, 0.2f, 0.4f)));
		ui.sceneView->addEntity(mBRDFObjectOutline, PR::UI::VL_Overlay1);
	} else {
		mBRDFObject->enableNormalization(ui.normCB->isChecked());
		mBRDFObject->rebuild();
	}

	if (!mBTDFObject) {
		mBTDFObject = std::make_shared<PR::UI::HemiFunctionEntity>([this](const Vector3f& D) { return evalBSDF(-D); });
		mBTDFObject->setScale(Vector3f(1.0f, 1.0f, -1.0f));
		mBTDFObject->enableNormalization(ui.normCB->isChecked());
		ui.sceneView->addEntity(mBTDFObject);

		// Wireframe
		mBTDFObjectOutline = std::make_shared<PR::UI::InstanceEntity>(mBTDFObject);
		mBTDFObjectOutline->setShader(std::make_shared<PR::UI::WireframeShader>(Vector4f(0.2f, 0.2f, 0.2f, 0.4f)));
		ui.sceneView->addEntity(mBTDFObjectOutline, PR::UI::VL_Overlay1);
	} else {
		mBTDFObject->enableNormalization(ui.normCB->isChecked());
		mBTDFObject->rebuild();
	}

	ui.createButton->setText(tr("Update"));
}

Vector3f MaterialWindow::generateL() const
{
	return mLightSource->translation().normalized();
}

float MaterialWindow::evalBSDF(const Vector3f& d) const
{
	RenderTileSession session; // Empty session
	MaterialEvalInput in;

	in.Context.V = d;
	in.Context.L = generateL();

	if (ui.swapCB->isChecked())
		std::swap(in.Context.V, in.Context.L);

	in.Context.P			= Vector3f::Zero();
	in.Context.UV			= Vector2f::Zero();
	in.Context.PrimitiveID	= 0;
	in.Context.WavelengthNM = SpectralBlob((float)ui.wavelengthSlider->value());

	in.ShadingContext = ShadingContext::fromMC(0, in.Context);

	MaterialEvalOutput out;
	mMaterial->eval(in, out, session);

	const float ndotl = std::abs(in.Context.L(2));
	if (ui.ndotlCB->isChecked() || ndotl <= PR_EPSILON)
		return out.Weight[0];
	else
		return out.Weight[0] / ndotl;
}