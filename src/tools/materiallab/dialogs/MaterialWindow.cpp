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

#include "3d/entities/GraphicEntity.h"

#include <QInputDialog>
#include <QMessageBox>

#include <fstream>

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
	ui.sceneView->repaint();
}

void MaterialWindow::populateInfo()
{
	QStringList names;
	for (auto name : mFactory->getNames())
		names.append(QString::fromStdString(name));
	ui.namesView->addItems(names);
}

template <typename T>
static inline T lerp(const T& a, const T& b, float t)
{
	return a * (1 - t) + b * t;
}

// t in [0,1]
static inline Vector3f colormap(float t)
{
	if (t >= 0.5f)
		return lerp(Vector3f(0, 1, 0), Vector3f(1, 0, 0), (t - 0.5f) * 2);
	else
		return lerp(Vector3f(0, 0, 1), Vector3f(0, 1, 0), t * 2);
}

void MaterialWindow::buildGraphicObjects()
{
	if (!mBRDFObject) {
		/*mBRDFObject = std::make_shared<PR::UI::GraphicObject>(true);
		mBRDFObject->setDrawMode(GL_TRIANGLES);
		ui.sceneView->addGraphicObject(mBRDFObject);*/
	}

	buildGraphicObject(mBRDFObject.get(), false);

	if (!mBTDFObject) {
		/*mBTDFObject = std::make_shared<PR::UI::GraphicObject>(true);
		mBTDFObject->setDrawMode(GL_TRIANGLES);
		ui.sceneView->addGraphicObject(mBTDFObject);*/
	}

	buildGraphicObject(mBTDFObject.get(), true);
}

void MaterialWindow::buildGraphicObject(PR::UI::GraphicEntity* bxdf, bool neg)
{
	PR_UNUSED(bxdf);
	PR_UNUSED(neg);
#if 0
	constexpr int Nradius = 32;
	constexpr int Nphi	  = 96;
	constexpr float R	  = 1.0f;

	float maxValue = 0;
	//bxdf->vertices().reserve((Nradius + 1) * (Nphi + 1));
	//bxdf->normals().resize(Nx * Ny);
	for (int j = 0; j <= Nradius; ++j) {
		for (int i = 0; i <= Nphi; ++i) {
			float r	  = std::sqrt(j / (float)Nradius);
			float phi = 2 * PR::PR_PI * (i / (float)Nphi);

			float x = r * std::cos(phi);
			float y = r * std::sin(phi);

			Vector3f D = Vector3f(x, y, std::sqrt(1 - r * r));
			if (neg)
				D = -D;

			float val = evalBSDF(D);
			if (!std::isfinite(val))
				val = 0;
			maxValue = std::max(maxValue, val);

			//bxdf->vertices().push_back(val * D);
		}
	}

	// Color
	//bxdf->colors().reserve((Nradius + 1) * (Nphi + 1));
	for (int j = 0; j <= Nradius; ++j) {
		for (int i = 0; i <= Nphi; ++i) {
			int ind = j * (Nphi + 1) + i;

			float val = 0;//bxdf->vertices()[ind].length();
			if (maxValue < 1)
				val /= maxValue;

			if (val <= 1)
				val *= 0.5f;
			else
				val = (val / maxValue) * 0.5f + 0.5f;

			//bxdf->colors().push_back(colormap(val));
		}
	}

	// Renormalize z
	/*for (auto& v : bxdf->vertices())
		v *= R / maxValue;*/

	// Indices
	//bxdf->indices().reserve(Nradius * Nphi * 3 * 2);
	for (int j = 0; j < Nradius; ++j) {
		for (int i = 0; i < Nphi; ++i) {
			uint32 row1 = j * (Nphi + 1);
			uint32 row2 = (j + 1) * (Nphi + 1);

			//bxdf->indices().append({ row1 + i, row1 + i + 1, row2 + i + 1 }); // Triangle 1
			//bxdf->indices().append({ row1 + i, row2 + i + 1, row2 + i });	  // Triangle 2
		}
	}
#endif

#if 0
	if (neg)
		return;
	std::ofstream stream("mesh.obj");
	stream << "o BSDF" << std::endl;
	for (const auto& v : bxdf->vertices())
		stream << "v " << v.x() << " " << v.y() << " " << v.z() << std::endl;
	for (int i = 0; i < bxdf->indices().size() / 3; ++i)
		stream << "f " << (bxdf->indices()[i * 3] + 1) << " " << (bxdf->indices()[i * 3 + 1] + 1) << " " << (bxdf->indices()[i * 3 + 2] + 1) << std::endl;
	stream.close();
#endif
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