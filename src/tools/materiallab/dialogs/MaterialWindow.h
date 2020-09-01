#pragma once

#include "ui_MaterialWindow.h"
#include <QWidget>

#include <memory>

namespace PR {
class IMaterialPlugin;
class IMaterial;
} // namespace PR

namespace PR {
namespace UI {
class PropertyContainer;
class GraphicEntity;
} // namespace UI
} // namespace PR

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
	void buildGraphicObject(PR::UI::GraphicEntity* bxdf, bool neg);
	float evalBSDF(const PR::Vector3f& d) const;

	Ui::MaterialWindowClass ui;
	const QString mTypeName;
	PR::IMaterialPlugin* mFactory;
	PR::UI::PropertyContainer* mProperties;

	std::shared_ptr<PR::IMaterial> mMaterial;

	std::shared_ptr<PR::UI::GraphicEntity> mBRDFObject;
	std::shared_ptr<PR::UI::GraphicEntity> mBTDFObject;
};