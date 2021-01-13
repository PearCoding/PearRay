#include "MeshParser.h"
#include "Logger.h"
#include "MathParser.h"
#include "mesh/MeshBase.h"

#include "DataLisp.h"

namespace PR {

template <int D>
static bool loadAttribute(const std::string& attrname, const DL::DataGroup& grp, std::vector<float>& attr)
{
	static_assert(D > 0 && D <= 3, "Invalid dimension given");

	attr.reserve(attr.size() + grp.anonymousCount() * D);

	for (size_t j = 0; j < grp.anonymousCount(); ++j) {
		DL::Data attrValD = grp.at(j);
		if (attrValD.type() != DL::DT_Group) {
			PR_LOG(L_ERROR) << "Mesh " << attrname << " attribute is invalid." << std::endl;
			return false;
		}

		bool ok;
		Vector3f v = MathParser::getVector(attrValD.getGroup(), ok);

		if (ok) {
			for (int i = 0; i < D; ++i)
				attr.push_back(v[i]);
		} else {
			PR_LOG(L_ERROR) << "Mesh " << attrname << " attribute entry is invalid." << std::endl;
			return false;
		}
	}
	return true;
}

static inline std::optional<std::pair<size_t, size_t>> countTriQuad(const DL::DataGroup& grp)
{
	size_t triCount	 = 0;
	size_t quadCount = 0;
	for (size_t j = 0; j < grp.anonymousCount(); ++j) {
		DL::Data iD = grp.at(j);

		if (iD.type() != DL::DT_Group) {
			PR_LOG(L_ERROR) << "Given face is invalid." << std::endl;
			return {};
		}

		DL::DataGroup arr = iD.getGroup();

		if (!arr.isArray()) {
			PR_LOG(L_ERROR) << "Given face data is invalid." << std::endl;
			return {};
		}

		if (!arr.isAllOfType(DL::DT_Integer)) {
			PR_LOG(L_ERROR) << "Given index is not integer." << std::endl;
			return {};
		}

		if (arr.anonymousCount() == 3) {
			++triCount;
		} else if (arr.anonymousCount() == 4) {
			++quadCount;
		} else {
			PR_LOG(L_ERROR) << "Only triangle or quad faces are supported." << std::endl;
			return {};
		}
	}

	return { { triCount, quadCount } };
}

// Setup indices, vertex component indices will be handled special
static bool setupIndices(MeshComponent component, MeshBase* me, const DL::DataGroup& grp)
{
	const size_t vertexCount = me->nodeCount();
	const auto triQuadC		 = countTriQuad(grp);
	if (!triQuadC.has_value())
		return false;

	const size_t triCount  = triQuadC.value().first;
	const size_t quadCount = triQuadC.value().second;

	bool needsVPF	 = (component == MeshComponent::Vertex) && triCount != 0 && quadCount != 0;
	size_t facecount = triCount + quadCount;

	std::vector<uint32> indices;
	indices.reserve(triCount * 3 + quadCount * 4);

	std::vector<uint8> verticesPerFace;
	if (needsVPF)
		verticesPerFace.reserve(facecount);

	for (size_t j = 0; j < grp.anonymousCount(); ++j) {
		DL::DataGroup arr = grp.at(j).getGroup();

		for (size_t d = 0; d < arr.anonymousCount(); ++d) {
			int32 val = static_cast<int32>(arr.at(d).getInt());

			if (val < 0 || (size_t)val >= vertexCount) {
				PR_LOG(L_ERROR) << "Given index range is invalid." << std::endl;
				return false;
			}
			indices.push_back(static_cast<uint32>(val));
		}

		if (needsVPF)
			verticesPerFace.push_back(static_cast<uint8>(grp.anonymousCount()));
	}

	me->setVertexComponentIndices(component, std::move(indices));

	// Set face vertex count buffer if we deal with the vertex indices
	if (component == MeshComponent::Vertex) {
		if (!verticesPerFace.empty())
			me->setFaceVertexCount(std::move(verticesPerFace));
		else {
			if (quadCount == 0)
				me->assumeTriangular(facecount);
			else if (triCount == 0)
				me->assumeQuadrangular(facecount);
			else {
				PR_ASSERT(false, "Should not be reached!");
			}
		}
	}

	return true;
}

std::unique_ptr<MeshBase> MeshParser::parse(const DL::DataGroup& group)
{
	auto me = std::make_unique<MeshBase>();

	// First get vertex attributes
	for (size_t i = 0; i < group.anonymousCount(); ++i) {
		DL::Data d = group.at(i);
		if (d.type() != DL::DT_Group) {
			PR_LOG(L_ERROR) << "Invalid entry in mesh description." << std::endl;
			return nullptr;
		}

		DL::DataGroup& grp = d.getGroup();
		if (grp.id() == "attribute") {
			DL::Data attrTypeD = grp.getFromKey("type");
			if (attrTypeD.type() != DL::DT_String) {
				PR_LOG(L_ERROR) << "Mesh attribute has no valid type." << std::endl;
				return nullptr;
			} else if (attrTypeD.getString() == "p") {
				std::vector<float> arr;
				if (!loadAttribute<3>("position", grp, arr))
					return nullptr;
				me->setVertexComponent(MeshComponent::Vertex, std::move(arr));
			} else if (attrTypeD.getString() == "n") {
				std::vector<float> arr;
				if (!loadAttribute<3>("normal", grp, arr))
					return nullptr;
				me->setVertexComponent(MeshComponent::Normal, std::move(arr));
			} else if (attrTypeD.getString() == "t" || attrTypeD.getString() == "uv") {
				std::vector<float> arr;
				if (!loadAttribute<2>("texture", grp, arr))
					return nullptr;
				me->setVertexComponent(MeshComponent::Texture, std::move(arr));
			} else if (attrTypeD.getString() == "w") {
				std::vector<float> arr;
				if (!loadAttribute<3>("weight", grp, arr))
					return nullptr;
				me->setVertexComponent(MeshComponent::Weight, std::move(arr));
			} else if (attrTypeD.getString() == "dp") {
				std::vector<float> arr;
				if (!loadAttribute<3>("velocity", grp, arr))
					return nullptr;
				me->setVertexComponent(MeshComponent::Velocity, std::move(arr));
			} else if (attrTypeD.getString() == "u") {
				PR_LOG(L_WARNING) << "User attributes currently not supported." << std::endl;
			} else {
				PR_LOG(L_ERROR) << "Unknown mesh attribute '" << attrTypeD.getString() << "'." << std::endl;
				return nullptr;
			}

			// Will not be used anywhere -> Clear for memory space
			grp.clear();
		}
	}

	// Get face attributes
	for (size_t i = 0; i < group.anonymousCount(); ++i) {
		DL::Data d = group.at(i);
		if (d.type() != DL::DT_Group) {
			PR_LOG(L_ERROR) << "Invalid entry in mesh description." << std::endl;
			return nullptr;
		}

		DL::DataGroup& grp = d.getGroup();
		if (grp.id() == "faces") {
			if (!setupIndices(MeshComponent::Vertex, me.get(), grp))
				return nullptr;
		} else if (grp.id() == "normal_faces") {
			if (!setupIndices(MeshComponent::Normal, me.get(), grp))
				return nullptr;
		} else if (grp.id() == "texture_faces") {
			if (!setupIndices(MeshComponent::Texture, me.get(), grp))
				return nullptr;
		} else if (grp.id() == "weight_faces") {
			if (!setupIndices(MeshComponent::Weight, me.get(), grp))
				return nullptr;
		} else if (grp.id() == "velocity_faces") {
			if (!setupIndices(MeshComponent::Velocity, me.get(), grp))
				return nullptr;
		} else if (grp.id() == "materials") {
			std::vector<uint32> materials;
			materials.reserve(grp.anonymousCount());
			for (size_t j = 0; j < grp.anonymousCount(); j++) {
				DL::Data indexD = grp.at(j);

				if (indexD.type() != DL::DT_Integer) {
					PR_LOG(L_ERROR) << "Given index is invalid." << std::endl;
					return nullptr;
				}

				auto index = indexD.getInt();

				if (index < 0) {
					PR_LOG(L_ERROR) << "Given index range is invalid." << std::endl;
					return nullptr;
				}

				materials.push_back(static_cast<uint32>(index));
			}
			me->setMaterialSlots(std::move(materials));
		}

		// Will not be used anywhere -> Clear for memory space
		grp.clear();
	}

	if (!me->isValid()) {
		PR_LOG(L_ERROR) << "Loaded mesh is invalid." << std::endl;
		return nullptr;
	}

	return me;
}
} // namespace PR
