#include "RendererService.h"
#include "Logger.h"

using namespace PR;
namespace PRO
{
	RendererService::RendererService()
	{
	}

	RendererService::~RendererService()
	{
	}

	bool RendererService::get_matrix(OSL::ShaderGlobals *sg, OSL::Matrix44 &result,
		OSL::TransformationPtr xform, float time)
	{
		//TODO
		return false;
	}
	
	bool RendererService::get_matrix(OSL::ShaderGlobals *sg, OSL::Matrix44 &result,
		OSL::TransformationPtr xform)
	{
		//TODO
		return false;
	}
	
	bool RendererService::get_matrix(OSL::ShaderGlobals *sg, OSL::Matrix44 &result,
		OSL::ustring from, float time)
	{
		//TODO
		return false;
	}
	
	bool RendererService::get_matrix(OSL::ShaderGlobals *sg, OSL::Matrix44 &result,
		OSL::ustring from)
	{
		//TODO
		return false;
	}

	bool RendererService::get_attribute(OSL::ShaderGlobals *sg, bool derivatives,
		OSL::ustring object, OSL::TypeDesc type, OSL::ustring name,
		void *val)
	{
		return get_array_attribute(sg, derivatives, object,
			type, name, -1, val);
	}
	
	bool RendererService::get_array_attribute(OSL::ShaderGlobals *sg, bool derivatives,
		OSL::ustring object, OSL::TypeDesc type,
		OSL::ustring name, int index, void *val)
	{
		//TODO
		return false;
	}
	
	bool RendererService::get_userdata(bool derivatives, OSL::ustring name, OSL::TypeDesc type,
		OSL::ShaderGlobals *sg, void *val)
	{
		//TODO
		return false;
	}
}