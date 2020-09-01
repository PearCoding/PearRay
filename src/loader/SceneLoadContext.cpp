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
	if (mEnvironment)
		return mEnvironment->lookupRawNode(parameter);
	else
		return nullptr;
}

std::shared_ptr<FloatSpectralNode> SceneLoadContext::lookupSpectralNode(const Parameter& parameter, float def) const
{
	if (mEnvironment)
		return mEnvironment->lookupSpectralNode(parameter, def);
	else
		return std::make_shared<ConstSpectralNode>(SpectralBlob(def));
}

std::shared_ptr<FloatSpectralNode> SceneLoadContext::lookupSpectralNode(const Parameter& parameter, const SpectralBlob& def) const
{
	if (mEnvironment)
		return mEnvironment->lookupSpectralNode(parameter, def);
	else
		return std::make_shared<ConstSpectralNode>(def);
}

std::shared_ptr<FloatScalarNode> SceneLoadContext::lookupScalarNode(const Parameter& parameter, float def) const
{
	if (mEnvironment)
		return mEnvironment->lookupScalarNode(parameter, def);
	else
		return std::make_shared<ConstScalarNode>(def);
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
