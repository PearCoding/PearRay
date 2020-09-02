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
#include "3d/shader/WireframeShader.h"

#include <QInputDialog>
#include <QMessageBox>

using namespace PR;

MaterialWindow::MaterialWindow(const QString& typeName, PR::IMaterialPlugin* factory, QWidget* parent)
	: QWidget(parent)
	, mTypeName(typeName)
	, mFactory(factory)
	, mProperties(new PR::UI::PropertyContainer())
{
	PR_ASSERT(mFactory, "Expected valid factory");

	ui.setupUi(this);

	// Read only checkboxes
	ui.deltaCB->setAttribute(Qt::WA_TransparentForMouseEvents);
	ui.deltaCB->setFocusPolicy(Qt::NoFocus);

	ui.spectralCB->setAttribute(Qt::WA_TransparentForMouseEvents);
	ui.spectralCB->setFocusPolicy(Qt::NoFocus);

	connect(ui.addPropertyButton, &QPushButton::clicked, this, &MaterialWindow::addProperty);
	connect(ui.createButton, &QPushButton::clicked, this, &MaterialWindow::createMaterial);

	setWindowTitle(mTypeName);

	ui.propertyView->setPropertyContainer(mProperties);

	ui.sceneView->enableAntialiasing();
	ui.sceneView->enableExtraEntities();

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

	int flags = mMaterial->flags();
	ui.deltaCB->setChecked(flags & PR::MF_DeltaDistribution);
	ui.spectralCB->setChecked(flags & PR::MF_SpectralVarying);

	buildGraphicObjects();
}

void MaterialWindow::populateInfo()
{
	QStringList names;
	for (auto name : mFactory->getNames())
		names.append(QString::fromStdString(name));
	ui.namesView->addItems(names);
}

void MaterialWindow::buildGraphicObjects()
{
	if (!mBRDFObject) {
		mBRDFObject = std::make_shared<PR::UI::HemiFunctionEntity>([this](const Vector3f& D) { return evalBSDF(D); });
		ui.sceneView->addEntity(mBRDFObject);

		// Wireframe
		mBRDFObjectOutline = std::make_shared<PR::UI::InstanceEntity>(mBRDFObject);
		mBRDFObjectOutline->setShader(std::make_shared<PR::UI::WireframeShader>(Vector4f(0.2f, 0.2f, 0.2f, 0.4f)));
		ui.sceneView->addEntity(mBRDFObjectOutline, PR::UI::VL_Overlay1);
	} else {
		mBRDFObject->rebuild();
	}

	if (!mBTDFObject) {
		mBTDFObject = std::make_shared<PR::UI::HemiFunctionEntity>([this](const Vector3f& D) { return evalBSDF(-D); });
		mBTDFObject->setScale(Vector3f(1.0f, 1.0f, -1.0f));
		ui.sceneView->addEntity(mBTDFObject);

		// Wireframe
		mBTDFObjectOutline = std::make_shared<PR::UI::InstanceEntity>(mBTDFObject);
		mBTDFObjectOutline->setShader(std::make_shared<PR::UI::WireframeShader>(Vector4f(0.2f, 0.2f, 0.2f, 0.4f)));
		ui.sceneView->addEntity(mBTDFObjectOutline, PR::UI::VL_Overlay1);
	} else {
		mBTDFObject->rebuild();
	}

	ui.createButton->setText(tr("Update"));
}

float MaterialWindow::evalBSDF(const Vector3f& d) const
{
	constexpr bool VIsInput	  = true;
	static const Vector3f Out = Vector3f(0, 1, 1).normalized();

	RenderTileSession session; // Empty session
	MaterialEvalInput in;

	in.Context.V = d;
	in.Context.L = Out;

	if (!VIsInput)
		std::swap(in.Context.V, in.Context.L);

	in.Context.P			= Vector3f::Zero();
	in.Context.UV			= Vector2f::Zero();
	in.Context.PrimitiveID	= 0;
	in.Context.WavelengthNM = SpectralBlob(510);

	in.ShadingContext = ShadingContext::fromMC(0, in.Context);

	MaterialEvalOutput out;
	mMaterial->eval(in, out, session);
	return out.Weight[0];
}