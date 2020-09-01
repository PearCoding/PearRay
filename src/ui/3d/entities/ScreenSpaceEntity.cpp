#include "ScreenSpaceEntity.h"
#include "3d/shader/BackgroundShader.h"

namespace PR {
namespace UI {
ScreenSpaceEntity::ScreenSpaceEntity()
	: GraphicEntity()
{
	useProxyVertexSize(6);

	std::shared_ptr<BackgroundShader> shader = std::make_shared<BackgroundShader>();
	setShader(shader);
}

ScreenSpaceEntity::~ScreenSpaceEntity()
{
}
} // namespace UI
} // namespace PR