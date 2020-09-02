#include "InstanceEntity.h"

namespace PR {
namespace UI {
InstanceEntity::InstanceEntity(const std::shared_ptr<GraphicEntity>& original)
	: GraphicEntity()
	, mOriginal(original)
{
}

InstanceEntity::~InstanceEntity() {}

void InstanceEntity::setOriginal(const std::shared_ptr<GraphicEntity>& original)
{
	mOriginal = original;
}

void InstanceEntity::render(const ShadingContext& sc)
{
	if (!mOriginal)
		return;

	const bool wasVisible	  = mOriginal->isVisible();
	const auto originalShader = mOriginal->shader();

	ShadingContext nsc = sc.chain(transform());

	if (shader())
		mOriginal->setShader(shader());

	mOriginal->show();
	mOriginal->render(nsc);
	mOriginal->show(wasVisible);

	if (shader())
		mOriginal->setShader(originalShader);
}
} // namespace UI
} // namespace PR