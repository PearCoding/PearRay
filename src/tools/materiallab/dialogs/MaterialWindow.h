#pragma once

#include "ui_MaterialWindow.h"
#include <QWidget>

#include <memory>

namespace PR {
class IMaterialPlugin;
class IMaterial;
} // namespace PR

namespace PRUI {
class PropertyContainer;
class GraphicObject;
} // namespace PRUI

class MaterialWindow : public QWidget {
	Q_OBJECT

public:
	MaterialWindow(const QString& typeName, PR::IMaterialPlugin* factory, QWidget* parent = 0);
	~MaterialWindow();

private slots:
	void addProperty();
	void createMaterial();

private:
	void populateInfo();
	void buildGraphicObjects();
	void buildGraphicObject(PRUI::GraphicObject* bxdf, bool neg);
	float evalBSDF(const QVector3D& d) const;

	Ui::MaterialWindowClass ui;
	const QString mTypeName;
	PR::IMaterialPlugin* mFactory;
	PRUI::PropertyContainer* mProperties;

	std::shared_ptr<PR::IMaterial> mMaterial;

	std::shared_ptr<PRUI::GraphicObject> mBRDFObject;
	std::shared_ptr<PRUI::GraphicObject> mBTDFObject;
};