#include "MeshParser.h"
#include "Logger.h"
#include "MathParser.h"
#include "mesh/MeshContainer.h"

#include "DataLisp.h"

namespace PR {

template <int D>
static bool loadAttribute(const std::string& attrname, const DL::DataGroup& grp, std::vector<float>* attr)
{
	static_assert(D > 0 && D <= 3, "Invalid dimension given");

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
				attr[i].push_back(v[i]);
		} else {
			PR_LOG(L_ERROR) << "Mesh " << attrname << " attribute entry is invalid." << std::endl;
			return false;
		}
	}

	return true;
}

std::shared_ptr<MeshContainer> MeshParser::parse(Environment*,
												 const DL::DataGroup& group)
{
	std::vector<float> positionAttr[3];
	std::vector<float> normalAttr[3];
	std::vector<float> velocityAttr[3];
	std::vector<float> uvAttr[2];
	// TODO: More attributes!

	// First get vertex attributes
	DL::DataGroup facesGrp;
	bool hasFaces = false;
	DL::DataGroup materialsGrp;
	bool hasMaterials = false;

	for (size_t i = 0; i < group.anonymousCount(); ++i) {
		DL::Data d = group.at(i);
		if (d.type() != DL::DT_Group) {
			PR_LOG(L_ERROR) << "Invalid entry in mesh description." << std::endl;
			return nullptr;
		}

		const DL::DataGroup& grp = d.getGroup();
		if (grp.id() == "attribute") {
			DL::Data attrTypeD = grp.getFromKey("type");
			if (attrTypeD.type() != DL::DT_String) {
				PR_LOG(L_ERROR) << "Mesh attribute has no valid type." << std::endl;
				return nullptr;
			} else if (attrTypeD.getString() == "p") {
				if (!loadAttribute<3>("position", grp, positionAttr))
					return nullptr;
			} else if (attrTypeD.getString() == "n") {
				if (!loadAttribute<3>("normal", grp, normalAttr))
					return nullptr;
			} else if (attrTypeD.getString() == "t") {
				if (!loadAttribute<2>("texture", grp, uvAttr))
					return nullptr;
			} else if (attrTypeD.getString() == "dp") {
				if (!loadAttribute<3>("velocity", grp, velocityAttr))
					return nullptr;
			} else if (attrTypeD.getString() == "u") {
				PR_LOG(L_WARNING) << "User attributes currently not supported." << std::endl;
			} else {
				PR_LOG(L_ERROR) << "Unknown mesh attribute." << std::endl;
				return nullptr;
			}
		} else if (grp.id() == "faces") {
			if (hasFaces) {
				PR_LOG(L_WARNING) << "Faces already set for mesh." << std::endl;
			} else {
				facesGrp = grp;
				hasFaces = true;
			}
		} else if (grp.id() == "materials") {
			if (hasMaterials) {
				PR_LOG(L_WARNING) << "Materials already set for mesh." << std::endl;
			} else {
				materialsGrp = grp;
				hasMaterials = true;
			}
		} else {
			PR_LOG(L_ERROR) << "Invalid entry in mesh description." << std::endl;
			return nullptr;
		}
	}

	// Check validity
	if (positionAttr[0].empty()) {
		PR_LOG(L_ERROR) << "No position attribute given." << std::endl;
		return nullptr;
	}

	if (normalAttr[0].empty() || normalAttr[0].size() != positionAttr[0].size()) {
		PR_LOG(L_ERROR) << "Normal attribute does not match position attribute in size or is empty." << std::endl;
		return nullptr;
	}

	if (!uvAttr[0].empty() && uvAttr[0].size() != positionAttr[0].size()) {
		PR_LOG(L_ERROR) << "Texture attribute does not match position attribute in size." << std::endl;
		return nullptr;
	}

	if (!velocityAttr[0].empty() && velocityAttr[0].size() != positionAttr[0].size()) {
		PR_LOG(L_ERROR) << "Velocity attribute does not match position attribute in size." << std::endl;
		return nullptr;
	}

	// Face attributes
	std::vector<uint32> materials;
	if (hasMaterials) {
		materials.reserve(materialsGrp.anonymousCount());
		for (size_t j = 0; j < materialsGrp.anonymousCount(); j++) {
			DL::Data indexD = materialsGrp.at(j);

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
	}

	// Get indices (faces)
	std::vector<uint32> indices;
	std::vector<uint8> verticesPerFace;
	const size_t vertexCount = positionAttr[0].size();
	if (!hasFaces) {
		PR_LOG(L_ERROR) << "No faces given!" << std::endl;
		return nullptr;
	} else {
		if (hasMaterials && materialsGrp.anonymousCount() != facesGrp.anonymousCount()) {
			PR_LOG(L_ERROR) << "Given material index count is not equal to face count." << std::endl;
			return nullptr;
		}

		size_t triCount  = 0;
		size_t quadCount = 0;
		for (size_t j = 0; j < facesGrp.anonymousCount(); ++j) {
			DL::Data iD = facesGrp.at(j);

			if (iD.type() != DL::DT_Group) {
				PR_LOG(L_ERROR) << "Given face is invalid." << std::endl;
				return nullptr;
			}

			DL::DataGroup grp = iD.getGroup();

			if (!grp.isArray()) {
				PR_LOG(L_ERROR) << "Given face data is invalid." << std::endl;
				return nullptr;
			}

			if (!grp.isAllOfType(DL::DT_Integer)) {
				PR_LOG(L_ERROR) << "Given index is not integer." << std::endl;
				return nullptr;
			}

			if (grp.anonymousCount() == 3) {
				++triCount;
			} else if (grp.anonymousCount() == 4) {
				++quadCount;
			} else {
				PR_LOG(L_ERROR) << "Only triangle or quad faces are supported." << std::endl;
				return nullptr;
			}
		}

		indices.reserve(triCount * 3 + quadCount * 4);
		verticesPerFace.reserve(facesGrp.anonymousCount());

		for (size_t j = 0; j < facesGrp.anonymousCount(); ++j) {
			DL::DataGroup grp = facesGrp.at(j).getGroup();

			for (size_t d = 0; d < grp.anonymousCount(); ++d) {
				int val = grp.at(d).getInt();

				if (val < 0 || (size_t)val >= vertexCount) {
					PR_LOG(L_ERROR) << "Given index range is invalid." << std::endl;
					return nullptr;
				}
				indices.push_back(static_cast<uint32>(val));
			}

			verticesPerFace.push_back(static_cast<uint8>(grp.anonymousCount()));
		}
	}

	auto me = std::make_shared<MeshContainer>();
	me->setVertices(positionAttr[0], positionAttr[1], positionAttr[2]);
	me->setNormals(normalAttr[0], normalAttr[1], normalAttr[2]);
	me->setUVs(uvAttr[0], uvAttr[1]);
	me->setVelocities(velocityAttr[0], velocityAttr[1], velocityAttr[2]);
	me->setMaterialSlots(materials);
	me->setIndices(indices);
	me->setFaceVertexCount(verticesPerFace);

	if (!me->isValid()) {
		PR_LOG(L_ERROR) << "Loaded mesh is invalid." << std::endl;
		return nullptr;
	}

	DL::Data triangulateD = group.getFromKey("triangulate");
	if (triangulateD.type() == DL::DT_Bool && triangulateD.getBool())
		me->triangulate();

	return me;
}
} // namespace PR
