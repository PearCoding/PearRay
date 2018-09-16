#pragma once

#include <QWidget>

namespace PR
{
	class VirtualEntity;
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
	void setEntity(PR::VirtualEntity* entity);

private slots:
	void propertyValueChanged(IProperty* prop);

private:
	void addRenderEntity();
	void addSphere();
	void addStdCamera();
	void addMesh();
	void addPointLight();
	void addPlane();

	PropertyView* mView;

	PR::VirtualEntity* mEntity;
	PropertyTable* mPropertyTable;
	QList<IProperty*> mProperties;
};
