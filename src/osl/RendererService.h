#pragma once

#include "PR_Config.h"
#include "PearMath.h"

#include <OSL/oslexec.h>

namespace PRO
{
	// TODO
	class PR_LIB_OSL RendererService : public OSL::RendererServices
	{
	public:
		RendererService();
		virtual ~RendererService();

		virtual bool get_matrix(OSL::ShaderGlobals *sg, OSL::Matrix44 &result,
			OSL::TransformationPtr xform, float time);
		virtual bool get_matrix(OSL::ShaderGlobals *sg, OSL::Matrix44 &result,
			OSL::TransformationPtr xform);
		virtual bool get_matrix(OSL::ShaderGlobals *sg, OSL::Matrix44 &result,
			OSL::ustring from, float time);
		virtual bool get_matrix(OSL::ShaderGlobals *sg, OSL::Matrix44 &result,
			OSL::ustring from);

		virtual bool get_attribute(OSL::ShaderGlobals *sg, bool derivatives,
			OSL::ustring object, OSL::TypeDesc type, OSL::ustring name,
			void *val);
		virtual bool get_array_attribute(OSL::ShaderGlobals *sg, bool derivatives,
			OSL::ustring object, OSL::TypeDesc type,
			OSL::ustring name, int index, void *val);
		virtual bool get_userdata(bool derivatives, OSL::ustring name, OSL::TypeDesc type,
			OSL::ShaderGlobals *sg, void *val);
	};
}
