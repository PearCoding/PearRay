#include "ScreenSpaceEntity.h"
#include "3d/shader/BackgroundShader.h"

namespace PR {
namespace UI {
ScreenSpaceEntity::ScreenSpaceEntity()
	: GraphicEntity()
{
	useProxyVertexSize(6);
	setShader(std::make_shared<BackgroundShader>());
}

ScreenSpaceEntity::~ScreenSpaceEntity()
{
}
} // namespace UI
} // namespace PR