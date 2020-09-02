#pragma once

#include "ui_MaterialWindow.h"
#include <QWidget>

#include <memory>

namespace PR {
class IMaterialPlugin;
class IMaterial;
namespace UI {
class PropertyContainer;
class HemiFunctionEntity;
class InstanceEntity;
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
	float evalBSDF(const PR::Vector3f& d) const;

	Ui::MaterialWindowClass ui;
	const QString mTypeName;
	PR::IMaterialPlugin* mFactory;
	PR::UI::PropertyContainer* mProperties;

	std::shared_ptr<PR::IMaterial> mMaterial;

	std::shared_ptr<PR::UI::HemiFunctionEntity> mBRDFObject;
	std::shared_ptr<PR::UI::InstanceEntity> mBRDFObjectOutline;
	std::shared_ptr<PR::UI::HemiFunctionEntity> mBTDFObject;
	std::shared_ptr<PR::UI::InstanceEntity> mBTDFObjectOutline;
};