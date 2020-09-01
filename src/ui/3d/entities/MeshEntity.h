#pragma once

#include "GraphicEntity.h"

namespace PR {
namespace UI {
class PR_LIB_UI MeshEntity : public GraphicEntity {
public:
	MeshEntity();
	virtual ~MeshEntity();

	void setMesh(const std::vector<float>& vertices, const std::vector<unsigned int>& indices, const std::vector<float>& normals);
};
} // namespace UI
}