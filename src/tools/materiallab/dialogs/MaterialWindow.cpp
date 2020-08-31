#include "MaterialWindow.h"
#include "SceneLoadContext.h"
#include "material/IMaterial.h"
#include "material/IMaterialPlugin.h"

#include "properties/BoolProperty.h"
#include "properties/DoubleProperty.h"
#include "properties/IntProperty.h"
#include "properties/PropertyContainer.h"
#include "properties/TextProperty.h"

#include <QInputDialog>
#include <QMessageBox>

MaterialWindow::MaterialWindow(const QString& typeName, PR::IMaterialPlugin* factory, QWidget* parent)
	: QWidget(parent)
	, mTypeName(typeName)
	, mFactory(factory)
	, mProperties(new PRUI::PropertyContainer())
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

	ui.sceneView->addAxis();

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
			ui.propertyView->addProperty(new PRUI::TextProperty());
		else if (item == "Bool")
			ui.propertyView->addProperty(new PRUI::BoolProperty());
		else if (item == "Integer")
			ui.propertyView->addProperty(new PRUI::IntProperty());
		else
			ui.propertyView->addProperty(new PRUI::DoubleProperty());
	}
}

void MaterialWindow::createMaterial()
{
	PR::SceneLoadContext ctx;
	for (auto prop : mProperties->topProperties()) {
		if (auto* p = dynamic_cast<PRUI::TextProperty*>(prop))
			ctx.Parameters.addParameter(p->propertyName().toStdString(), PR::Parameter::fromString(p->text().toStdString()));
		else if (auto* p = dynamic_cast<PRUI::BoolProperty*>(prop))
			ctx.Parameters.addParameter(p->propertyName().toStdString(), PR::Parameter::fromBool(p->value()));
		else if (auto* p = dynamic_cast<PRUI::IntProperty*>(prop))
			ctx.Parameters.addParameter(p->propertyName().toStdString(), PR::Parameter::fromInt(p->value()));
		else if (auto* p = dynamic_cast<PRUI::DoubleProperty*>(prop))
			ctx.Parameters.addParameter(p->propertyName().toStdString(), PR::Parameter::fromNumber(p->value()));
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
}

void MaterialWindow::populateInfo()
{
	QStringList names;
	for (auto name : mFactory->getNames())
		names.append(QString::fromStdString(name));
	ui.namesView->addItems(names);
}
