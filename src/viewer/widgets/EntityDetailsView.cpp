#include "EntityDetailsView.h"
#include "PropertyView.h"

#include "entity/Entity.h"
#include "entity/RenderEntity.h"
#include "entity/SphereEntity.h"
#include "entity/MeshEntity.h"
#include "entity/GridEntity.h"

#include "camera/OrthographicCamera.h"
#include "camera/PerspectiveCamera.h"

#include "properties/propertytable.h"

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

void EntityDetailsView::setEntity(PR::Entity* entity)
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

		GroupProperty* group = new GroupProperty();
		group->setPropertyName(tr("Entity"));
		mProperties.append(group);

		TextProperty* name = new TextProperty();
		name->setPropertyName(tr("Name"));
		name->setText(entity->name().c_str());
		name->setReadOnly(true);
		group->addChild(name);
		mProperties.append(name);

		TextProperty* type = new TextProperty();
		type->setPropertyName(tr("Type"));
		type->setText(QString(entity->type().c_str()).toUpper());
		type->setReadOnly(true);
		group->addChild(type);
		mProperties.append(type);

		VectorProperty* pos = new VectorProperty();
		pos->setPropertyName(tr("Position"));
		pos->setDefaultValue(PM::pm_GetX(entity->position(true)), 1);
		pos->setDefaultValue(PM::pm_GetY(entity->position(true)), 2);
		pos->setDefaultValue(PM::pm_GetZ(entity->position(true)), 3);
		pos->setValue(PM::pm_GetX(entity->position(true)), 1);
		pos->setValue(PM::pm_GetY(entity->position(true)), 2);
		pos->setValue(PM::pm_GetZ(entity->position(true)), 3);
		group->addChild(pos);
		mProperties.append(pos);

		VectorProperty* rot = new VectorProperty(4);
		rot->setPropertyName(tr("Rotation"));
		rot->setDefaultValue(PM::pm_GetX(entity->rotation(true)), 1);
		rot->setDefaultValue(PM::pm_GetY(entity->rotation(true)), 2);
		rot->setDefaultValue(PM::pm_GetZ(entity->rotation(true)), 3);
		rot->setDefaultValue(PM::pm_GetW(entity->rotation(true)), 4);
		rot->setValue(PM::pm_GetX(entity->rotation(true)), 1);
		rot->setValue(PM::pm_GetY(entity->rotation(true)), 2);
		rot->setValue(PM::pm_GetZ(entity->rotation(true)), 3);
		rot->setValue(PM::pm_GetW(entity->rotation(true)), 4);
		group->addChild(rot);
		mProperties.append(rot);

		VectorProperty* sca = new VectorProperty();
		sca->setPropertyName(tr("Scale"));
		sca->setDefaultValue(PM::pm_GetX(entity->scale(true)), 1);
		sca->setDefaultValue(PM::pm_GetY(entity->scale(true)), 2);
		sca->setDefaultValue(PM::pm_GetZ(entity->scale(true)), 3);
		sca->setValue(PM::pm_GetX(entity->scale(true)), 1);
		sca->setValue(PM::pm_GetY(entity->scale(true)), 2);
		sca->setValue(PM::pm_GetZ(entity->scale(true)), 3);
		group->addChild(sca);
		mProperties.append(sca);

		mPropertyTable->add(group);

		if (entity->type() == "sphere")
		{
			addSphere();
		}
		else if (entity->type() == "orthographicCamera")
		{
			addOrthoCamera();
		}
		else if (entity->type() == "perspectiveCamera")
		{
			addPersCamera();
		}
		else if (entity->type() == "pointLight")
		{
			addPointLight();
		}
		else if (entity->type() == "grid")
		{
			addGrid();
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

	PR::RenderEntity* e = (PR::RenderEntity*)mEntity;
	//Nothing
}

void EntityDetailsView::addSphere()
{
	Q_ASSERT(mEntity);

	PR::SphereEntity* e = (PR::SphereEntity*)mEntity;

	GroupProperty* group = new GroupProperty();
	group->setPropertyName(tr("Sphere"));
	mProperties.append(group);

	DoubleProperty* rad = new DoubleProperty();
	rad->setPropertyName(tr("Radius"));
	rad->setDefaultValue(e->radius());
	rad->setValue(e->radius());
	rad->setMinValue(0);
	group->addChild(rad);
	mProperties.append(rad);

	mPropertyTable->add(group);
}

void EntityDetailsView::addOrthoCamera()
{
	Q_ASSERT(mEntity);

	PR::OrthographicCamera* cam = (PR::OrthographicCamera*)mEntity;

	GroupProperty* group = new GroupProperty();
	group->setPropertyName(tr("Orthographic Camera"));
	mProperties.append(group);

	DoubleProperty* w = new DoubleProperty();
	w->setPropertyName(tr("Width"));
	w->setDefaultValue(cam->width());
	w->setValue(cam->width());
	w->setMinValue(0);
	group->addChild(w);
	mProperties.append(w);

	DoubleProperty* h = new DoubleProperty();
	h->setPropertyName(tr("Height"));
	h->setDefaultValue(cam->height());
	h->setValue(cam->height());
	h->setMinValue(0);
	group->addChild(h);
	mProperties.append(h);

	DoubleProperty* l = new DoubleProperty();
	l->setPropertyName(tr("Lens Distance"));
	l->setDefaultValue(cam->lensDistance());
	l->setValue(cam->lensDistance());
	l->setMinValue(0);
	group->addChild(l);
	mProperties.append(l);

	mPropertyTable->add(group);
}

void EntityDetailsView::addPersCamera()
{
	Q_ASSERT(mEntity);

	PR::PerspectiveCamera* cam = (PR::PerspectiveCamera*)mEntity;

	GroupProperty* group = new GroupProperty();
	group->setPropertyName(tr("Perspective Camera"));
	mProperties.append(group);

	DoubleProperty* w = new DoubleProperty();
	w->setPropertyName(tr("Width"));
	w->setDefaultValue(cam->width());
	w->setValue(cam->width());
	w->setMinValue(0);
	group->addChild(w);
	mProperties.append(w);

	DoubleProperty* h = new DoubleProperty();
	h->setPropertyName(tr("Height"));
	h->setDefaultValue(cam->height());
	h->setValue(cam->height());
	h->setMinValue(0);
	group->addChild(h);
	mProperties.append(h);

	DoubleProperty* l = new DoubleProperty();
	l->setPropertyName(tr("Lens Distance"));
	l->setDefaultValue(cam->lensDistance());
	l->setValue(cam->lensDistance());
	l->setMinValue(0);
	group->addChild(l);
	mProperties.append(l);

	mPropertyTable->add(group);
}

void EntityDetailsView::addPointLight()
{
	Q_ASSERT(mEntity);
	
	GroupProperty* group = new GroupProperty();
	group->setPropertyName(tr("Point Light"));
	mProperties.append(group);

	mPropertyTable->add(group);
}

void EntityDetailsView::addMesh()
{
	Q_ASSERT(mEntity);

	GroupProperty* group = new GroupProperty();
	group->setPropertyName(tr("Mesh"));
	mProperties.append(group);

	mPropertyTable->add(group);
}

void EntityDetailsView::addGrid()
{
	Q_ASSERT(mEntity);

	GroupProperty* group = new GroupProperty();
	group->setPropertyName(tr("Grid"));
	mProperties.append(group);

	PR::GridEntity* ent = (PR::GridEntity*)mEntity;

	VectorProperty* xAxis = new VectorProperty();
	xAxis->setPropertyName(tr("XAxis"));
	xAxis->setDefaultValue(PM::pm_GetX(ent->plane().xAxis()), 1);
	xAxis->setDefaultValue(PM::pm_GetY(ent->plane().xAxis()), 2);
	xAxis->setDefaultValue(PM::pm_GetZ(ent->plane().xAxis()), 3);
	xAxis->setValue(PM::pm_GetX(ent->plane().xAxis()), 1);
	xAxis->setValue(PM::pm_GetY(ent->plane().xAxis()), 2);
	xAxis->setValue(PM::pm_GetZ(ent->plane().xAxis()), 3);
	group->addChild(xAxis);
	mProperties.append(xAxis);

	VectorProperty* yAxis = new VectorProperty();
	yAxis->setPropertyName(tr("YAxis"));
	yAxis->setDefaultValue(PM::pm_GetX(ent->plane().yAxis()), 1);
	yAxis->setDefaultValue(PM::pm_GetY(ent->plane().yAxis()), 2);
	yAxis->setDefaultValue(PM::pm_GetZ(ent->plane().yAxis()), 3);
	yAxis->setValue(PM::pm_GetX(ent->plane().yAxis()), 1);
	yAxis->setValue(PM::pm_GetY(ent->plane().yAxis()), 2);
	yAxis->setValue(PM::pm_GetZ(ent->plane().yAxis()), 3);
	group->addChild(yAxis);
	mProperties.append(yAxis);

	IntProperty* gridCount = new IntProperty();
	gridCount->setPropertyName(tr("Grid Count"));
	gridCount->setDefaultValue(ent->gridCount());
	gridCount->setValue(ent->gridCount());
	gridCount->setMinValue(1);
	group->addChild(gridCount);
	mProperties.append(gridCount);

	mPropertyTable->add(group);
}

// FIXME: Using strings as identification is not that good.
void EntityDetailsView::propertyValueChanged(IProperty* prop)
{
	Q_ASSERT(mEntity);

	if (prop->propertyName() == tr("Position"))
	{
		VectorProperty* p = (VectorProperty*)prop;
		mEntity->setPosition(PM::pm_Set(p->value(1), p->value(2), p->value(3)));
	}
	else if (prop->propertyName() == tr("Rotation"))
	{
		VectorProperty* p = (VectorProperty*)prop;
		mEntity->setRotation(PM::pm_Set(p->value(1), p->value(2), p->value(3), p->value(4)));
	}
	else if (prop->propertyName() == tr("Scale"))
	{
		VectorProperty* p = (VectorProperty*)prop;
		mEntity->setScale(PM::pm_Set(p->value(1), p->value(2), p->value(3)));
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

		if (mEntity->type() == "orthographicCamera")
		{
			((PR::OrthographicCamera*)mEntity)->setWidth(p->value());
		}
		else if (mEntity->type() == "perspectiveCamera")
		{
			((PR::PerspectiveCamera*)mEntity)->setWidth(p->value());
		}
	}
	else if (prop->propertyName() == tr("Height"))
	{
		DoubleProperty* p = (DoubleProperty*)prop;

		if (mEntity->type() == "orthographicCamera")
		{
			((PR::OrthographicCamera*)mEntity)->setHeight(p->value());
		}
		else if (mEntity->type() == "perspectiveCamera")
		{
			((PR::PerspectiveCamera*)mEntity)->setHeight(p->value());
		}
	}
	else if (prop->propertyName() == tr("Lens Distance"))
	{
		DoubleProperty* p = (DoubleProperty*)prop;

		if (mEntity->type() == "orthographicCamera")
		{
			((PR::OrthographicCamera*)mEntity)->setLensDistance(p->value());
		}
		else if (mEntity->type() == "perspectiveCamera")
		{
			((PR::PerspectiveCamera*)mEntity)->setLensDistance(p->value());
		}
	}
}