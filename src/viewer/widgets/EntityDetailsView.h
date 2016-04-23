#pragma once

#include <QWidget>

namespace PR
{
	class Entity;
}

class IProperty;
class PropertyTable;
class PropertyView;
class EntityDetailsView : public QWidget
{
	Q_OBJECT
public:
	explicit EntityDetailsView(QWidget* parent = nullptr);
	virtual ~EntityDetailsView();

public slots:
	void setEntity(PR::Entity* entity);

private slots:
	void propertyValueChanged(IProperty* prop);

private:
	void addRenderEntity();
	void addSphere();
	void addOrthoCamera();
	void addPersCamera();
	void addMesh();
	void addPointLight();
	void addGrid();

	PropertyView* mView;

	PR::Entity* mEntity;
	PropertyTable* mPropertyTable;
	QList<IProperty*> mProperties;
};
