#pragma once

#include "MeshInlineParser.h"

namespace PRU
{
	class TriMeshInlineParser : public IMeshInlineParser
	{
	public:
		PR::IMesh* parse(SceneLoader* loader, Environment* env, DL::DataGroup* group) const override;
	};
}
