#include "EntityDetailsView.h"
#include "PropertyView.h"

#include "entity/Entity.h"
#include "entity/RenderEntity.h"
#include "entity/SphereEntity.h"
#include "entity/MeshEntity.h"

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
		else if (entity->type() == "mesh")
		{
			addMesh();
		}
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
