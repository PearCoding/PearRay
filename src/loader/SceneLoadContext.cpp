#include "SceneLoadContext.h"
#include "Environment.h"
#include "ResourceManager.h"
#include "filter/FilterManager.h"
#include "integrator/IntegratorManager.h"
#include "parser/TextureParser.h"
#include "sampler/SamplerManager.h"
#include "shader/ConstNode.h"
#include "spectral/SpectralMapperManager.h"

namespace PR {

SceneLoadContext::SceneLoadContext(Environment* env)
	: mParameters()
	, mTransform(Transformf::Identity())
	, mFileStack()
	, mEnvironment(env)
{
}

SceneLoadContext::SceneLoadContext(const std::filesystem::path& filename)
	: mParameters()
	, mTransform(Transformf::Identity())
	, mFileStack()
	, mEnvironment(nullptr)
{
	mFileStack.push_back(filename);
}

SceneLoadContext::SceneLoadContext(Environment* env, const std::filesystem::path& filename)
	: mParameters()
	, mTransform(Transformf::Identity())
	, mFileStack()
	, mEnvironment(env)
{
	mFileStack.push_back(filename);
}

bool SceneLoadContext::hasFile(const std::filesystem::path& filename) const
{
	return std::find(mFileStack.begin(), mFileStack.end(), filename) != mFileStack.end();
}

void SceneLoadContext::pushFile(const std::filesystem::path& filename)
{
	mFileStack.push_back(filename);
}

void SceneLoadContext::popFile()
{
	PR_ASSERT(mFileStack.size() >= 1, "Invalid push/pop count");
	mFileStack.pop_back();
}

std::filesystem::path SceneLoadContext::setupParametricImage(const std::filesystem::path& path)
{
	bool updateNeeded	 = false;
	const auto para_path = mEnvironment->resourceManager()->requestFile("image", path.stem(), ".exr", updateNeeded);

	if (updateNeeded) {
		PR_LOG(L_INFO) << "Converting " << path << " to parametric image " << para_path << std::endl;
		TextureParser::convertToParametric(*this, path, para_path);
	}

	return para_path;
}

std::shared_ptr<INode> SceneLoadContext::lookupRawNode(const Parameter& parameter) const
{
	switch (parameter.type()) {
	default:
		return nullptr;
	case PT_Int:
	case PT_UInt:
	case PT_Number:
		if (parameter.isArray()) {
			if (parameter.arraySize() == 2)
				return std::make_shared<ConstVectorNode>(
					Vector3f(parameter.getNumber(0, 0.0f), parameter.getNumber(1, 0.0f), 0.0f));
			else if (parameter.arraySize() == 3)
				return std::make_shared<ConstVectorNode>(
					Vector3f(parameter.getNumber(0, 0.0f), parameter.getNumber(1, 0.0f), parameter.getNumber(2, 0.0f)));
		} else {
			return std::make_shared<ConstScalarNode>(parameter.getNumber(0.0f));
		}
		break;
	case PT_Reference:
		if (hasEnvironment())
			return mEnvironment->getRawNode(parameter.getReference());
		break;
	case PT_String:
		if (hasEnvironment())
			mEnvironment->getRawNode(parameter.getString(""));
		break;
	}
	return nullptr;
}

std::shared_ptr<FloatSpectralNode> SceneLoadContext::lookupSpectralNode(const Parameter& parameter, float def) const
{
	return lookupSpectralNode(parameter, SpectralBlob(def));
}

std::shared_ptr<FloatSpectralNode> SceneLoadContext::lookupSpectralNode(const Parameter& parameter, const SpectralBlob& def) const
{
	switch (parameter.type()) {
	default:
		return std::make_shared<ConstSpectralNode>(def);
	case PT_Int:
	case PT_UInt:
	case PT_Number:
		if (parameter.isArray())
			return std::make_shared<ConstSpectralNode>(def);
		else
			return std::make_shared<ConstSpectralNode>(SpectralBlob(parameter.getNumber(0.0f)));
	case PT_Reference:
		if (hasEnvironment()) {
			auto node = mEnvironment->getRawNode(parameter.getReference());
			if (node->type() == NT_FloatSpectral)
				return std::reinterpret_pointer_cast<FloatSpectralNode>(node);
			else if (node->type() == NT_FloatScalar)
				return std::make_shared<SplatSpectralNode>(std::reinterpret_pointer_cast<FloatScalarNode>(node));
		}
		return std::make_shared<ConstSpectralNode>(def);
	case PT_String:
		if (hasEnvironment()) {
			std::string name = parameter.getString("");

			if (mEnvironment->hasNode(name)) {
				auto socket = mEnvironment->getNode<FloatSpectralNode>(name);
				if (socket)
					return socket;
			}
		}
		return std::make_shared<ConstSpectralNode>(def);
	}
}

std::shared_ptr<FloatScalarNode> SceneLoadContext::lookupScalarNode(const Parameter& parameter, float def) const
{
	switch (parameter.type()) {
	default:
		return std::make_shared<ConstScalarNode>(def);
	case PT_Int:
	case PT_UInt:
	case PT_Number:
		if (parameter.isArray())
			return std::make_shared<ConstScalarNode>(def);
		else
			return std::make_shared<ConstScalarNode>(parameter.getNumber(def));
	case PT_Reference:
		if (hasEnvironment()) {
			auto node = mEnvironment->getRawNode(parameter.getReference());
			if (node->type() == NT_FloatScalar)
				return std::reinterpret_pointer_cast<FloatScalarNode>(node);
		}
		return std::make_shared<ConstScalarNode>(def);
	case PT_String:
		if (hasEnvironment()) {
			std::string name = parameter.getString("");

			if (mEnvironment->hasNode(name)) {
				auto socket = mEnvironment->getNode<FloatScalarNode>(name);
				if (socket)
					return socket;
			}
		}
		return std::make_shared<ConstScalarNode>(def);
	}
}

std::shared_ptr<INode> SceneLoadContext::lookupRawNode(const std::string& parameter) const
{
	return lookupRawNode(mParameters.getParameter(parameter));
}

std::shared_ptr<FloatSpectralNode> SceneLoadContext::lookupSpectralNode(const std::string& parameter, float def) const
{
	return lookupSpectralNode(mParameters.getParameter(parameter), def);
}

std::shared_ptr<FloatSpectralNode> SceneLoadContext::lookupSpectralNode(const std::string& parameter, const SpectralBlob& def) const
{
	return lookupSpectralNode(mParameters.getParameter(parameter), def);
}

std::shared_ptr<FloatScalarNode> SceneLoadContext::lookupScalarNode(const std::string& parameter, float def) const
{
	return lookupScalarNode(mParameters.getParameter(parameter), def);
}

std::shared_ptr<IIntegratorFactory> SceneLoadContext::loadIntegratorFactory(const std::string& type, const ParameterGroup& params) const
{
	auto manag		= mEnvironment->integratorManager();
	const uint32 id = manag->nextID();

	auto fac = manag->getFactory(type);
	if (!fac) {
		PR_LOG(L_ERROR) << "Unknown integrator type " << type << std::endl;
		return nullptr;
	}

	SceneLoadContext copy = *this;
	copy.mParameters	  = params;
	return fac->create(id, type, copy);
}

std::shared_ptr<ISamplerFactory> SceneLoadContext::loadSamplerFactory(const std::string& type, const ParameterGroup& params) const
{
	auto manag		= mEnvironment->samplerManager();
	const uint32 id = manag->nextID();

	auto fac = manag->getFactory(type);
	if (!fac) {
		PR_LOG(L_ERROR) << "Unknown sampler type " << type << std::endl;
		return nullptr;
	}

	SceneLoadContext copy = *this;
	copy.mParameters	  = params;
	return fac->create(id, type, copy);
}

std::shared_ptr<IFilterFactory> SceneLoadContext::loadFilterFactory(const std::string& type, const ParameterGroup& params) const
{
	auto manag		= mEnvironment->filterManager();
	const uint32 id = manag->nextID();

	auto fac = manag->getFactory(type);
	if (!fac) {
		PR_LOG(L_ERROR) << "Unknown filter type " << type << std::endl;
		return nullptr;
	}

	SceneLoadContext copy = *this;
	copy.mParameters	  = params;
	return fac->create(id, type, copy);
}

std::shared_ptr<ISpectralMapperFactory> SceneLoadContext::loadSpectralMapperFactory(const std::string& type, const ParameterGroup& params) const
{
	auto manag		= mEnvironment->spectralMapperManager();
	const uint32 id = manag->nextID();

	auto fac = manag->getFactory(type);
	if (!fac) {
		PR_LOG(L_ERROR) << "Unknown spectral mapper type " << type << std::endl;
		return nullptr;
	}

	SceneLoadContext copy = *this;
	copy.mParameters	  = params;
	return fac->create(id, type, copy);
}
} // namespace PR
