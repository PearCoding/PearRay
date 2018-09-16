#include "EntityDetailsView.h"
#include "PropertyView.h"

#include "entity/VirtualEntity.h"
#include "entity/RenderEntity.h"
#include "entity/SphereEntity.h"
#include "entity/MeshEntity.h"
#include "entity/PlaneEntity.h"

#include "camera/StandardCamera.h"

#include "properties/PropertyTable.h"

#include "properties/GroupProperty.h"
#include "properties/IntProperty.h"
#include "properties/DoubleProperty.h"
#include "properties/ButtonProperty.h"
#include "properties/BoolProperty.h"
#include "properties/SelectionProperty.h"
#include "properties/TextProperty.h"
#include "properties/VectorProperty.h"

#include <QBoxLayout>

EntityDetailsView::EntityDetailsView(QWidget* parent) :
QWidget(parent),
mEntity(nullptr), mPropertyTable(nullptr)
{
	mView = new PropertyView(this);
	
	QHBoxLayout* layout = new QHBoxLayout(this);
	layout->addWidget(mView);
	layout->setSpacing(0);
	layout->setContentsMargins(0, 0, 0, 0);

	setLayout(layout);
}

EntityDetailsView::~EntityDetailsView()
{
	if (mPropertyTable)
	{
		delete mPropertyTable;
		qDeleteAll(mProperties);
	}
}

void EntityDetailsView::setEntity(PR::VirtualEntity* entity)
{
	if (mPropertyTable)
	{
		mView->setPropertyTable(nullptr);

		mPropertyTable->disconnect();
		delete mPropertyTable;
		mPropertyTable = nullptr;

		qDeleteAll(mProperties);
		mProperties.clear();
	}
	mEntity = entity;

	if (entity)
	{
		mPropertyTable = new PropertyTable;

		GroupProperty* group = new GroupProperty(tr("VirtualEntity"));
		mProperties.append(group);

		TextProperty* name = new TextProperty(tr("Name"), entity->name().c_str());
		name->setReadOnly(true);
		group->addChild(name);
		mProperties.append(name);

		TextProperty* type = new TextProperty(tr("Type"), QString(entity->type().c_str()).toUpper());
		type->setReadOnly(true);
		group->addChild(type);
		mProperties.append(type);

		VectorProperty* pos = new VectorProperty(tr("Position"));
		pos->setDefaultValue(entity->position()(0), 1);
		pos->setDefaultValue(entity->position()(1), 2);
		pos->setDefaultValue(entity->position()(2), 3);
		pos->setValue(entity->position()(0), 1);
		pos->setValue(entity->position()(1), 2);
		pos->setValue(entity->position()(2), 3);
		group->addChild(pos);
		mProperties.append(pos);

		VectorProperty* rot = new VectorProperty(tr("Rotation"), 4);
		rot->setDefaultValue(entity->rotation().x(), 1);
		rot->setDefaultValue(entity->rotation().y(), 2);
		rot->setDefaultValue(entity->rotation().z(), 3);
		rot->setDefaultValue(entity->rotation().w(), 4);
		rot->setValue(entity->rotation().x(), 1);
		rot->setValue(entity->rotation().y(), 2);
		rot->setValue(entity->rotation().z(), 3);
		rot->setValue(entity->rotation().w(), 4);
		group->addChild(rot);
		mProperties.append(rot);

		VectorProperty* sca = new VectorProperty(tr("Scale"), 3);
		sca->setDefaultValue(entity->scale()(0), 1);
		sca->setDefaultValue(entity->scale()(1), 2);
		sca->setDefaultValue(entity->scale()(2), 3);
		sca->setValue(entity->scale()(0), 1);
		sca->setValue(entity->scale()(1), 2);
		sca->setValue(entity->scale()(2), 3);
		group->addChild(sca);
		mProperties.append(sca);

		mPropertyTable->add(group);

		if (entity->type() == "sphere")
		{
			addSphere();
		}
		else if (entity->type() == "standardCamera")
		{
			addStdCamera();
		}
		else if (entity->type() == "pointLight")
		{
			addPointLight();
		}
		else if (entity->type() == "plane")
		{
			addPlane();
		}
		else if (entity->type() == "mesh")
		{
			addMesh();
		}

		connect(mPropertyTable, SIGNAL(valueChanged(IProperty*)), this, SLOT(propertyValueChanged(IProperty*)));
	}

	mView->setPropertyTable(mPropertyTable);
	mView->expandToDepth(1);
}

void EntityDetailsView::addRenderEntity()
{
	Q_ASSERT(mEntity);

	//PR::RenderEntity* e = (PR::RenderEntity*)mEntity;
	//Nothing
}

void EntityDetailsView::addSphere()
{
	Q_ASSERT(mEntity);

	PR::SphereEntity* e = (PR::SphereEntity*)mEntity;

	GroupProperty* group = new GroupProperty(tr("Sphere"));
	mProperties.append(group);

	DoubleProperty* rad = new DoubleProperty(tr("Radius"), e->radius(), 0);
	group->addChild(rad);
	mProperties.append(rad);

	mPropertyTable->add(group);
}

void EntityDetailsView::addStdCamera()
{
	Q_ASSERT(mEntity);

	PR::StandardCamera* cam = (PR::StandardCamera*)mEntity;

	GroupProperty* group = new GroupProperty(tr("Standard Camera"));
	mProperties.append(group);

	DoubleProperty* w = new DoubleProperty(tr("Width"), cam->width(), 0);
	group->addChild(w);
	mProperties.append(w);

	DoubleProperty* h = new DoubleProperty(tr("Height"), cam->height(), 0);
	group->addChild(h);
	mProperties.append(h);

	BoolProperty* o = new BoolProperty(tr("Orthographic"), cam->isOrthographic());
	group->addChild(o);
	mProperties.append(o);

	DoubleProperty* f = new DoubleProperty(tr("FStop"), cam->fstop(), 0);
	group->addChild(f);
	mProperties.append(f);

	DoubleProperty* a = new DoubleProperty(tr("Aperture Radius"), cam->apertureRadius(), 0);
	group->addChild(a);
	mProperties.append(a);

	mPropertyTable->add(group);
}

void EntityDetailsView::addPointLight()
{
	Q_ASSERT(mEntity);
	
	GroupProperty* group = new GroupProperty(tr("Point Light"));
	mProperties.append(group);

	mPropertyTable->add(group);
}

void EntityDetailsView::addMesh()
{
	Q_ASSERT(mEntity);

	GroupProperty* group = new GroupProperty(tr("Mesh"));
	mProperties.append(group);

	mPropertyTable->add(group);
}

void EntityDetailsView::addPlane()
{
	Q_ASSERT(mEntity);

	GroupProperty* group = new GroupProperty(tr("Grid"));
	mProperties.append(group);

	PR::PlaneEntity* ent = (PR::PlaneEntity*)mEntity;

	VectorProperty* xAxis = new VectorProperty(tr("XAxis"));
	xAxis->setDefaultValue(ent->plane().xAxis()(0), 1);
	xAxis->setDefaultValue(ent->plane().xAxis()(1), 2);
	xAxis->setDefaultValue(ent->plane().xAxis()(2), 3);
	xAxis->setValue(ent->plane().xAxis()(0), 1);
	xAxis->setValue(ent->plane().xAxis()(1), 2);
	xAxis->setValue(ent->plane().xAxis()(2), 3);
	group->addChild(xAxis);
	mProperties.append(xAxis);

	VectorProperty* yAxis = new VectorProperty(tr("YAxis"));
	yAxis->setDefaultValue(ent->plane().yAxis()(0), 1);
	yAxis->setDefaultValue(ent->plane().yAxis()(1), 2);
	yAxis->setDefaultValue(ent->plane().yAxis()(2), 3);
	yAxis->setValue(ent->plane().yAxis()(0), 1);
	yAxis->setValue(ent->plane().yAxis()(1), 2);
	yAxis->setValue(ent->plane().yAxis()(2), 3);
	group->addChild(yAxis);
	mProperties.append(yAxis);

	mPropertyTable->add(group);
}

// FIXME: Using strings as identification is not that good.
void EntityDetailsView::propertyValueChanged(IProperty* prop)
{
	Q_ASSERT(mEntity);

	if (prop->propertyName() == tr("Position"))
	{
		VectorProperty* p = (VectorProperty*)prop;
		mEntity->setPosition(Eigen::Vector3f(p->value(1), p->value(2), p->value(3)));
	}
	else if (prop->propertyName() == tr("Rotation"))
	{
		VectorProperty* p = (VectorProperty*)prop;
		mEntity->setRotation(Eigen::Quaternionf(p->value(1), p->value(2), p->value(3), p->value(4)));
	}
	else if (prop->propertyName() == tr("Scale"))
	{
		VectorProperty* p = (VectorProperty*)prop;
		mEntity->setScale(Eigen::Vector3f(p->value(1), p->value(2), p->value(3)));
	}
	// Sphere
	else if (prop->propertyName() == tr("Radius"))
	{
		DoubleProperty* p = (DoubleProperty*)prop;
		((PR::SphereEntity*)mEntity)->setRadius(p->value());
	}
	// Camera
	else if (prop->propertyName() == tr("Width"))
	{
		DoubleProperty* p = (DoubleProperty*)prop;

		((PR::StandardCamera*)mEntity)->setWidth(p->value());
	}
	else if (prop->propertyName() == tr("Height"))
	{
		DoubleProperty* p = (DoubleProperty*)prop;

		((PR::StandardCamera*)mEntity)->setHeight(p->value());
	}
	else if (prop->propertyName() == tr("Orthographic"))
	{
		BoolProperty* p = (BoolProperty*)prop;

		((PR::StandardCamera*)mEntity)->setOrthographic(p->value());
	}
	else if (prop->propertyName() == tr("FStop"))
	{
		DoubleProperty* p = (DoubleProperty*)prop;

		((PR::StandardCamera*)mEntity)->setFStop(p->value());
	}
	else if (prop->propertyName() == tr("Aperture Radius"))
	{
		DoubleProperty* p = (DoubleProperty*)prop;

		((PR::StandardCamera*)mEntity)->setApertureRadius(p->value());
	}
}