#include "SceneLoadContext.h"
#include "Environment.h"
#include "shader/ConstNode.h"

namespace PR {

SceneLoadContext::SceneLoadContext(Environment* env)
	: mParameters()
	, mFileStack()
	, mEnvironment(env)
{
}

SceneLoadContext::SceneLoadContext(const std::filesystem::path& filename)
	: mParameters()
	, mFileStack()
	, mEnvironment(nullptr)
{
	mFileStack.push_back(filename);
}

SceneLoadContext::SceneLoadContext(Environment* env, const std::filesystem::path& filename)
	: mParameters()
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

} // namespace PR
