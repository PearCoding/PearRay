#include "SceneLoadContext.h"
#include "Environment.h"
#include "ResourceManager.h"
#include "emission/EmissionManager.h"
#include "filter/FilterManager.h"
#include "integrator/IntegratorManager.h"
#include "material/IMaterial.h"
#include "material/MaterialManager.h"
#include "parser/TextureParser.h"
#include "sampler/SamplerManager.h"
#include "scene/SceneDatabase.h"
#include "shader/ConstNode.h"
#include "spectral/SpectralMapperManager.h"

namespace PR {

SceneLoadContext::SceneLoadContext(Environment* env, const std::filesystem::path& filename)
	: mParameters()
	, mTransform(Transformf::Identity())
	, mFileStack()
	, mEnvironment(env)
{
	PR_ASSERT(env, "Expected valid environment");

	if (!filename.empty())
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
	const auto para_path = mEnvironment->resourceManager()->requestFile("image", path.stem().generic_string(), ".exr", updateNeeded);

	if (updateNeeded) {
		PR_LOG(L_INFO) << "Converting " << path << " to parametric image " << para_path << std::endl;
		TextureParser::convertToParametric(*this, path, para_path);
	}

	return para_path;
}

std::shared_ptr<IEmission> SceneLoadContext::getEmission(const std::string& name) const
{
	return mEnvironment->sceneDatabase()->Emissions->get(name);
}

bool SceneLoadContext::hasEmission(const std::string& name) const
{
	return mEnvironment->sceneDatabase()->Emissions->has(name);
}

uint32 SceneLoadContext::addEmission(const std::string& name, const std::shared_ptr<IEmission>& mat)
{
	PR_ASSERT(mat, "Given emission has to be valid");
	PR_ASSERT(!hasEmission(name), "Given name should be unique");
	return mEnvironment->sceneDatabase()->Emissions->add(name, mat);
}

size_t SceneLoadContext::emissionCount() const
{
	return mEnvironment->sceneDatabase()->Emissions->size();
}

std::shared_ptr<IMaterial> SceneLoadContext::getMaterial(const std::string& name) const
{
	return mEnvironment->sceneDatabase()->Materials->get(name);
}

bool SceneLoadContext::hasMaterial(const std::string& name) const
{
	return mEnvironment->sceneDatabase()->Materials->has(name);
}

uint32 SceneLoadContext::addMaterial(const std::string& name, const std::shared_ptr<IMaterial>& mat)
{
	PR_ASSERT(mat, "Given material has to be valid");
	PR_ASSERT(!hasMaterial(name), "Given name should be unique");
	return mEnvironment->sceneDatabase()->Materials->add(name, mat);
}

size_t SceneLoadContext::materialCount() const
{
	return mEnvironment->sceneDatabase()->Materials->size();
}

std::shared_ptr<MeshBase> SceneLoadContext::getMesh(const std::string& name) const
{
	return hasMesh(name) ? mMeshes.at(name) : nullptr;
}

bool SceneLoadContext::hasMesh(const std::string& name) const
{
	return mMeshes.count(name) != 0;
}

void SceneLoadContext::addMesh(const std::string& name, const std::shared_ptr<MeshBase>& m)
{
	PR_ASSERT(m, "Given mesh has to be valid");
	PR_ASSERT(!hasMesh(name), "Given name should be unique");
	mMeshes.emplace(name, m);
}

uint32 SceneLoadContext::addNode(const std::string& name, const std::shared_ptr<INode>& output)
{
	PR_ASSERT(!hasNode(name), "Given name should be unique");
	return mEnvironment->sceneDatabase()->Nodes->add(name, output);
}

std::shared_ptr<INode> SceneLoadContext::getRawNode(uint32 id) const
{
	return mEnvironment->sceneDatabase()->Nodes->get(id);
}

std::shared_ptr<INode> SceneLoadContext::getRawNode(const std::string& name) const
{
	return mEnvironment->sceneDatabase()->Nodes->get(name);
}

void SceneLoadContext::getNode(const std::string& name, std::shared_ptr<FloatScalarNode>& node2) const
{
	const auto node = getRawNode(name);
	if (node->type() == NodeType::FloatScalar)
		node2 = std::reinterpret_pointer_cast<FloatScalarNode>(node);
	else
		node2 = nullptr;
}

void SceneLoadContext::getNode(const std::string& name, std::shared_ptr<FloatSpectralNode>& node2) const
{
	const auto node = getRawNode(name);
	if (node->type() == NodeType::FloatSpectral)
		node2 = std::reinterpret_pointer_cast<FloatSpectralNode>(node);
	else
		node2 = nullptr;
}

void SceneLoadContext::getNode(const std::string& name, std::shared_ptr<FloatVectorNode>& node2) const
{
	const auto node = getRawNode(name);
	if (node->type() == NodeType::FloatVector)
		node2 = std::reinterpret_pointer_cast<FloatVectorNode>(node);
	else
		node2 = nullptr;
}

bool SceneLoadContext::hasNode(const std::string& name) const
{
	return mEnvironment->sceneDatabase()->Nodes->has(name);
}

std::shared_ptr<INode> SceneLoadContext::lookupRawNode(const Parameter& parameter) const
{
	switch (parameter.type()) {
	default:
		return nullptr;
	case ParameterType::Int:
	case ParameterType::UInt:
	case ParameterType::Number:
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
	case ParameterType::Reference:
		return getRawNode(parameter.getReference());
		break;
	case ParameterType::String:
		return getRawNode(parameter.getString(""));
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
	case ParameterType::Int:
	case ParameterType::UInt:
	case ParameterType::Number:
		if (parameter.isArray())
			return std::make_shared<ConstSpectralNode>(def);
		else
			return std::make_shared<ConstSpectralNode>(SpectralBlob(parameter.getNumber(0.0f)));
	case ParameterType::Reference: {
		const auto node = getRawNode(parameter.getReference());
		if (node->type() == NodeType::FloatSpectral)
			return std::reinterpret_pointer_cast<FloatSpectralNode>(node);
		else if (node->type() == NodeType::FloatScalar)
			return std::make_shared<SplatSpectralNode>(std::reinterpret_pointer_cast<FloatScalarNode>(node));
		else
			return std::make_shared<ConstSpectralNode>(def);
	}
	case ParameterType::String: {
		const std::string name = parameter.getString("");

		if (hasNode(name)) {
			auto socket = getNode<FloatSpectralNode>(name);
			if (socket)
				return socket;
		}

		return std::make_shared<ConstSpectralNode>(def);
	}
	}
}

std::shared_ptr<FloatScalarNode> SceneLoadContext::lookupScalarNode(const Parameter& parameter, float def) const
{
	switch (parameter.type()) {
	default:
		return std::make_shared<ConstScalarNode>(def);
	case ParameterType::Int:
	case ParameterType::UInt:
	case ParameterType::Number:
		if (parameter.isArray())
			return std::make_shared<ConstScalarNode>(def);
		else
			return std::make_shared<ConstScalarNode>(parameter.getNumber(def));
	case ParameterType::Reference: {
		const auto node = getRawNode(parameter.getReference());
		if (node->type() == NodeType::FloatScalar)
			return std::reinterpret_pointer_cast<FloatScalarNode>(node);
		else
			return std::make_shared<ConstScalarNode>(def);
	}
	case ParameterType::String: {
		const std::string name = parameter.getString("");

		if (hasNode(name)) {
			auto socket = getNode<FloatScalarNode>(name);
			if (socket)
				return socket;
		}
		return std::make_shared<ConstScalarNode>(def);
	}
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

std::shared_ptr<INode> SceneLoadContext::lookupRawNode(const std::initializer_list<std::string>& parameters) const
{
	PR_ASSERT(parameters.begin() != parameters.end(), "Expected at least one parameter");
	for (const auto& p : parameters) {
		if (mParameters.hasParameter(p))
			return lookupRawNode(mParameters.getParameter(p));
	}
	return lookupRawNode(mParameters.getParameter(*parameters.begin()));
}

std::shared_ptr<FloatSpectralNode> SceneLoadContext::lookupSpectralNode(const std::initializer_list<std::string>& parameters, float def) const
{
	PR_ASSERT(parameters.begin() != parameters.end(), "Expected at least one parameter");
	for (const auto& p : parameters) {
		if (mParameters.hasParameter(p))
			return lookupSpectralNode(mParameters.getParameter(p), def);
	}
	return lookupSpectralNode(mParameters.getParameter(*parameters.begin()), def);
}

std::shared_ptr<FloatSpectralNode> SceneLoadContext::lookupSpectralNode(const std::initializer_list<std::string>& parameters, const SpectralBlob& def) const
{
	PR_ASSERT(parameters.begin() != parameters.end(), "Expected at least one parameter");
	for (const auto& p : parameters) {
		if (mParameters.hasParameter(p))
			return lookupSpectralNode(mParameters.getParameter(p), def);
	}
	return lookupSpectralNode(mParameters.getParameter(*parameters.begin()), def);
}

std::shared_ptr<FloatScalarNode> SceneLoadContext::lookupScalarNode(const std::initializer_list<std::string>& parameters, float def) const
{
	PR_ASSERT(parameters.begin() != parameters.end(), "Expected at least one parameter");
	for (const auto& p : parameters) {
		if (mParameters.hasParameter(p))
			return lookupScalarNode(mParameters.getParameter(p), def);
	}
	return lookupScalarNode(mParameters.getParameter(*parameters.begin()), def);
}

std::shared_ptr<IIntegratorFactory> SceneLoadContext::loadIntegratorFactory(const std::string& type, const ParameterGroup& params) const
{
	auto manag = mEnvironment->integratorManager();

	auto fac = manag->getFactory(type);
	if (!fac) {
		PR_LOG(L_ERROR) << "Unknown integrator type " << type << std::endl;
		return nullptr;
	}

	SceneLoadContext copy = *this;
	copy.mParameters	  = params;
	return fac->create(type, copy);
}

std::shared_ptr<ISamplerFactory> SceneLoadContext::loadSamplerFactory(const std::string& type, const ParameterGroup& params) const
{
	auto manag = mEnvironment->samplerManager();

	auto fac = manag->getFactory(type);
	if (!fac) {
		PR_LOG(L_ERROR) << "Unknown sampler type " << type << std::endl;
		return nullptr;
	}

	SceneLoadContext copy = *this;
	copy.mParameters	  = params;
	return fac->create(type, copy);
}

std::shared_ptr<IFilterFactory> SceneLoadContext::loadFilterFactory(const std::string& type, const ParameterGroup& params) const
{
	auto manag = mEnvironment->filterManager();

	auto fac = manag->getFactory(type);
	if (!fac) {
		PR_LOG(L_ERROR) << "Unknown filter type " << type << std::endl;
		return nullptr;
	}

	SceneLoadContext copy = *this;
	copy.mParameters	  = params;
	return fac->create(type, copy);
}

std::shared_ptr<ISpectralMapperFactory> SceneLoadContext::loadSpectralMapperFactory(const std::string& type, const ParameterGroup& params) const
{
	auto manag = mEnvironment->spectralMapperManager();

	auto fac = manag->getFactory(type);
	if (!fac) {
		PR_LOG(L_ERROR) << "Unknown spectral mapper type " << type << std::endl;
		return nullptr;
	}

	SceneLoadContext copy = *this;
	copy.mParameters	  = params;
	return fac->create(type, copy);
}

std::shared_ptr<IMaterial> SceneLoadContext::lookupMaterial(const Parameter& parameter) const
{
	switch (parameter.type()) {
	default:
		return nullptr;
	case ParameterType::Reference: // TODO
		return nullptr;
	case ParameterType::String:
		return getMaterial(parameter.getString(""));
	}
}

uint32 SceneLoadContext::lookupMaterialID(const Parameter& parameter) const
{
	switch (parameter.type()) {
	default:
		return PR_INVALID_ID;
	case ParameterType::Reference: // TODO
		return PR_INVALID_ID;
	case ParameterType::String:
		return environment()->sceneDatabase()->Materials->getID(parameter.getString(""));
	}
}

std::vector<uint32> SceneLoadContext::lookupMaterialIDArray(const Parameter& parameter, bool skipInvalid) const
{
	if (parameter.isArray()) {
		std::vector<uint32> mats;
		mats.reserve(parameter.arraySize());

		for (size_t i = 0; i < parameter.arraySize(); ++i) {
			uint32 id;
			switch (parameter.type()) {
			default:
				id = PR_INVALID_ID;
				break;
			case ParameterType::Reference: // TODO
				id = PR_INVALID_ID;
				break;
			case ParameterType::String:
				id = environment()->sceneDatabase()->Materials->getID(parameter.getString(i, ""));
				break;
			}

			if (id != PR_INVALID_ID || !skipInvalid)
				mats.push_back(id);
		}

		return mats;
	} else {
		return { lookupMaterialID(parameter) };
	}
}

std::shared_ptr<IEmission> SceneLoadContext::lookupEmission(const Parameter& parameter) const
{
	switch (parameter.type()) {
	default:
		return nullptr;
	case ParameterType::Reference: // TODO
		return nullptr;
	case ParameterType::String:
		return getEmission(parameter.getString(""));
	}
}

uint32 SceneLoadContext::lookupEmissionID(const Parameter& parameter) const
{
	switch (parameter.type()) {
	default:
		return PR_INVALID_ID;
	case ParameterType::Reference: // TODO
		return PR_INVALID_ID;
	case ParameterType::String:
		return environment()->sceneDatabase()->Emissions->getID(parameter.getString(""));
	}
}

std::shared_ptr<IMaterial> SceneLoadContext::registerMaterial(const std::string& name, const std::string& type, const ParameterGroup& params)
{
	auto mat = loadMaterial(type, params);
	if (!mat)
		return nullptr;

	addMaterial(name, mat);

	return mat;
}

std::shared_ptr<IMaterial> SceneLoadContext::loadMaterial(const std::string& type, const ParameterGroup& params) const
{
	auto manag = mEnvironment->materialManager();
	auto fac   = manag->getFactory(type);
	if (!fac) {
		PR_LOG(L_ERROR) << "Unknown material type " << type << std::endl;
		return nullptr;
	}

	SceneLoadContext copy = *this;
	copy.mParameters	  = params;
	auto mat			  = fac->create(type, copy);
	if (!mat) {
		PR_LOG(L_ERROR) << "Could not create material of type " << type << std::endl;
		return nullptr;
	}

	// None of the following is really working... but maybe in the future
	mat->enableShading(params.getBool("shadeable", mat->canBeShaded()));
	mat->enableShadow(params.getBool("shadow", mat->allowsShadow()));
	mat->enableSelfShadow(params.getBool("self_shadow", mat->allowsSelfShadow()));
	mat->enableCameraVisibility(params.getBool("camera_visible", mat->isCameraVisible()));

	return mat;
}

std::shared_ptr<IEmission> SceneLoadContext::registerEmission(const std::string& name, const std::string& type, const ParameterGroup& params)
{
	auto ems = loadEmission(type, params);
	if (!ems)
		return nullptr;

	addEmission(name, ems);

	return ems;
}

std::shared_ptr<IEmission> SceneLoadContext::loadEmission(const std::string& type, const ParameterGroup& params) const
{
	auto manag = mEnvironment->emissionManager();
	auto fac   = manag->getFactory(type);
	if (!fac) {
		PR_LOG(L_ERROR) << "Unknown emission type " << type << std::endl;
		return nullptr;
	}

	SceneLoadContext copy = *this;
	copy.mParameters	  = params;
	auto obj			  = fac->create(type, copy);
	if (!obj) {
		PR_LOG(L_ERROR) << "Could not create emission of type " << type << std::endl;
		return nullptr;
	}

	return obj;
}
} // namespace PR
