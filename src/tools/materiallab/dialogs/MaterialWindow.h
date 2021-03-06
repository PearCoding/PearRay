#pragma once

#include "ui_MaterialWindow.h"
#include <QTimer>
#include <QWidget>

#include <memory>

#include "MaterialCache.h"

namespace PR {
class IMaterialPlugin;
class IMaterial;
class Environment;

namespace UI {
class PropertyContainer;
class GraphicEntity;
//class PointMapEntity;
class GridMapEntity;
class InstanceEntity;
} // namespace UI
} // namespace PR

class MaterialWindow : public QWidget {
	Q_OBJECT

public:
	MaterialWindow(const QString& typeName, const std::shared_ptr<PR::Environment>& env,
				   PR::IMaterialPlugin* factory, QWidget* parent = 0);
	~MaterialWindow();

private slots:
	void addProperty();
	void createMaterial();

	void controlChanged();
	void displayChanged();
	void animationHandler();

private:
	PR::Vector3f generateL() const;

	void setupProperties();
	void populateInfo();
	void calculateBSDF();
	void buildGraphicObjects();
	void buildBRDF();
	void buildBTDF();
	float evalBSDF(const PR::Vector3f& d) const;

	Ui::MaterialWindowClass ui;
	const QString mTypeName;
	std::shared_ptr<PR::Environment> mEnv;
	PR::IMaterialPlugin* mFactory;
	PR::UI::PropertyContainer* mProperties;

	std::shared_ptr<PR::IMaterial> mMaterial;

	std::shared_ptr<PR::UI::GraphicEntity> mLightSource;

	//MaterialCache mBRDFCache;
	std::vector<float> mBRDFCache;
	//MaterialCache mBTDFCache;
	std::vector<float> mBTDFCache;

	std::shared_ptr<PR::UI::GridMapEntity> mBRDFObject;
	std::shared_ptr<PR::UI::InstanceEntity> mBRDFObjectOutline;
	std::shared_ptr<PR::UI::GridMapEntity> mBTDFObject;
	std::shared_ptr<PR::UI::InstanceEntity> mBTDFObjectOutline;

	QTimer mAnimationTimer;
};