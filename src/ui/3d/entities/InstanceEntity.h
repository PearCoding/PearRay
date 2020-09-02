#pragma once

#include "GraphicEntity.h"

namespace PR {
namespace UI {
class PR_LIB_UI InstanceEntity : public GraphicEntity {
public:
	InstanceEntity(const std::shared_ptr<GraphicEntity>& original);
	virtual ~InstanceEntity();

	void setOriginal(const std::shared_ptr<GraphicEntity>& original);
	inline std::shared_ptr<GraphicEntity> original() const { return mOriginal; }

	virtual void render(const ShadingContext& sc) override;

private:
	std::shared_ptr<GraphicEntity> mOriginal;
};
} // namespace UI
} // namespace PR