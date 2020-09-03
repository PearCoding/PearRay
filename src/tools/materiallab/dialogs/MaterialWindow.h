#pragma once

#include "ui_MaterialWindow.h"
#include <QTimer>
#include <QWidget>

#include <memory>

#include "MaterialCache.h"

namespace PR {
class IMaterialPlugin;
class IMaterial;
namespace UI {
class PropertyContainer;
class GraphicEntity;
class PointMapEntity;
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

	void controlChanged();
	void displayChanged();
	void animationHandler();

private:
	PR::Vector3f generateL() const;

	void populateInfo();
	void calculateBSDF();
	void buildGraphicObjects();
	void buildBRDF();
	void buildBTDF();
	float evalBSDF(const PR::Vector3f& d) const;

	Ui::MaterialWindowClass ui;
	const QString mTypeName;
	PR::IMaterialPlugin* mFactory;
	PR::UI::PropertyContainer* mProperties;

	std::shared_ptr<PR::IMaterial> mMaterial;

	std::shared_ptr<PR::UI::GraphicEntity> mLightSource;

	MaterialCache mBRDFCache;
	MaterialCache mBTDFCache;

	std::shared_ptr<PR::UI::PointMapEntity> mBRDFObject;
	std::shared_ptr<PR::UI::InstanceEntity> mBRDFObjectOutline;
	std::shared_ptr<PR::UI::PointMapEntity> mBTDFObject;
	std::shared_ptr<PR::UI::InstanceEntity> mBTDFObjectOutline;

	QTimer mAnimationTimer;
};