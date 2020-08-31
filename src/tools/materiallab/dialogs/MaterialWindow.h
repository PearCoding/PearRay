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
}

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

	Ui::MaterialWindowClass ui;
	const QString mTypeName;
	PR::IMaterialPlugin* mFactory;
	PRUI::PropertyContainer* mProperties;

	std::shared_ptr<PR::IMaterial> mMaterial;
};