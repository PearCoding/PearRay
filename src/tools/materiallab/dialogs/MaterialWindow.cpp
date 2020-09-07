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

#include "3d/entities/InstanceEntity.h"
//#include "3d/entities/PointMapEntity.h"
#include "3d/entities/GridMapEntity.h"
#include "3d/entities/SphereEntity.h"
#include "3d/shader/ColorShader.h"
#include "3d/shader/WireframeShader.h"

#include "Random.h"
#include "math/Sampling.h"

#include <QInputDialog>
#include <QMessageBox>

using namespace PR;

constexpr float SUN_R	   = 0.1f;
constexpr float SUN_D	   = 4.0f;
constexpr int ANIM_MS	   = 200;
constexpr int STITCH_COUNT = 32;
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

constexpr int THETA_COUNT = 64;
constexpr int PHI_COUNT	  = 128;

void MaterialWindow::calculateBSDF()
{
	mBRDFCache.clear();
	mBTDFCache.clear();

	const auto pipeline = [this](auto calc) {
#if 0 /* Deterministic calculation instead */
		constexpr PR::uint64 SEED = 181; // 42th prime number
		constexpr int ITRATIONS	  = 5000;
		PR::Random rnd(SEED);

#if 1 /* Will be removed when 3d triangulation finally works */
		constexpr int STICH_COUNT = 16;
		for (int i = 0; i < STICH_COUNT; ++i) {
			const float theta = 0.5f * PR::PR_PI * (i / (float(STICH_COUNT - 1)));
			calc(theta, 0);
			calc(theta, 2 * PR::PR_PI);
		}
#endif

		// Sample step
		for (int i = 0; i < ITRATIONS; ++i) {
			const float theta = 0.5f * PR::PR_PI * rnd.getFloat();
			const float phi	  = 2 * PR::PR_PI * rnd.getFloat();
			calc(theta, phi);
		}
#else
		for (int i = 0; i < THETA_COUNT; ++i) {
			for (int j = 0; j < PHI_COUNT; ++j) {
				const float theta = 0.5f * PR::PR_PI * i / float(THETA_COUNT - 1);
				const float phi	  = 2 * PR::PR_PI * j / float(PHI_COUNT - 1);
				calc(theta, phi);
			}
		}
#endif
	};

	if (ui.brdfCB->isChecked()) {
		pipeline([this](float theta, float phi) {
			const Vector3f D = Spherical::cartesian(theta, phi);
			const float val	 = evalBSDF(D);

			//mBRDFCache.addValue(theta, phi, val);
			mBRDFCache.emplace_back(val);
		});

		if (ui.normCB->isChecked()) {
#if 0
			mBRDFCache.normalizeMaximum();
#else
			float m = *std::max_element(mBRDFCache.begin(), mBRDFCache.end());
			if (std::abs(m) >= PR_EPSILON) {
				float im = 1 / m;
				for (auto& v : mBRDFCache)
					v *= im;
			}
#endif
		}
	}

	if (ui.btdfCB->isChecked()) {
		pipeline([this](float theta, float phi) {
			const Vector3f D = Spherical::cartesian(theta, phi);
			const float val	 = evalBSDF(-D);

			//mBTDFCache.addValue(theta, phi, val);
			mBTDFCache.emplace_back(val);
		});

		if (ui.normCB->isChecked()) {
#if 0
			mBTDFCache.normalizeMaximum();
#else
			float m = *std::max_element(mBTDFCache.begin(), mBTDFCache.end());
			if (std::abs(m) >= PR_EPSILON) {
				float im = 1 / m;
				for (auto& v : mBTDFCache)
					v *= im;
			}
#endif
		}
	}
}

void MaterialWindow::buildGraphicObjects()
{
	calculateBSDF();

	if (!mBRDFObject) {
		//mBRDFObject = std::make_shared<PR::UI::PointMapEntity>(PR::UI::PointMapEntity::MT_Spherical);
		mBRDFObject = std::make_shared<PR::UI::GridMapEntity>(PR::UI::GridMapEntity::MT_Spherical);
		ui.sceneView->addEntity(mBRDFObject);

		// Wireframe
		mBRDFObjectOutline = std::make_shared<PR::UI::InstanceEntity>(mBRDFObject);
		mBRDFObjectOutline->setShader(std::make_shared<PR::UI::WireframeShader>(Vector4f(0.2f, 0.2f, 0.2f, 0.4f)));
		ui.sceneView->addEntity(mBRDFObjectOutline, PR::UI::VL_Overlay1);
	}
	buildBRDF();

	if (!mBTDFObject) {
		mBTDFObject = std::make_shared<PR::UI::GridMapEntity>(PR::UI::GridMapEntity::MT_Spherical);
		mBTDFObject->setScale(Vector3f(1.0f, 1.0f, -1.0f));
		ui.sceneView->addEntity(mBTDFObject);

		// Wireframe
		mBTDFObjectOutline = std::make_shared<PR::UI::InstanceEntity>(mBTDFObject);
		mBTDFObjectOutline->setShader(std::make_shared<PR::UI::WireframeShader>(Vector4f(0.2f, 0.2f, 0.2f, 0.4f)));
		ui.sceneView->addEntity(mBTDFObjectOutline, PR::UI::VL_Overlay1);
	}
	buildBTDF();

	ui.createButton->setText(tr("Update"));
}

inline static void cleanup(std::vector<Vector2f>& pts, std::vector<float>& vals)
{
	constexpr float EPS = 0.0001f;
	auto it2			= vals.begin();
	for (auto it = pts.begin(); it != pts.end();) {
		bool needsCleanup = false;

		auto cit2 = it2 + 1;
		for (auto cit = it + 1; cit != pts.cend();) {
			if ((*it - *cit).squaredNorm() < EPS) {
				needsCleanup = true;
				break;
			}

			++cit;
			++cit2;
		}

		if (needsCleanup) {
			it	= pts.erase(it);
			it2 = vals.erase(it2);
		} else {
			++it;
			++it2;
		}
	}
}

void MaterialWindow::buildBRDF()
{
#if 0
	std::vector<Vector2f> points;
	std::vector<float> values;
	mBRDFCache.get(points, values);
	cleanup(points, values);
	mBRDFObject->build(points, values);
#else
	mBRDFObject->build(PHI_COUNT, mBRDFCache);
#endif
}

void MaterialWindow::buildBTDF()
{
#if 0
	std::vector<Vector2f> points;
	std::vector<float> values;
	mBTDFCache.get(points, values);
	cleanup(points, values);
	mBTDFObject->build(points, values);
#else
	mBTDFObject->build(PHI_COUNT, mBTDFCache);
#endif
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