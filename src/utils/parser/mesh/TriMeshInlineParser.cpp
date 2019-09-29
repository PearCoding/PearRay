#include "TriMeshInlineParser.h"
#include "Environment.h"
#include "Logger.h"
#include "SceneLoader.h"
#include "geometry/Face.h"
#include "mesh/TriMesh.h"

#include "DataLisp.h"

namespace PR {
std::shared_ptr<TriMesh> TriMeshInlineParser::parse(Environment* env, const DL::DataGroup& group) const
{
	std::vector<float> positionAttr[3];
	std::vector<float> normalAttr[3];
	std::vector<float> uvAttr[2];
	// TODO: More attributes!

	// First get attributes
	DL::DataGroup facesGrp;
	bool hasFaces = false;
	DL::DataGroup materialsGrp;
	bool hasMaterials = false;

	for (size_t i = 0; i < group.anonymousCount(); ++i) {
		DL::Data d = group.at(i);
		if (d.type() != DL::Data::T_Group) {
			PR_LOG(L_ERROR) << "Invalid entry in mesh description." << std::endl;
			return nullptr;
		}

		const DL::DataGroup& grp = d.getGroup();
		if (grp.id() == "attribute") {
			DL::Data attrTypeD = grp.getFromKey("type");
			if (attrTypeD.type() != DL::Data::T_String) {
				PR_LOG(L_ERROR) << "Mesh attribute has no valid type." << std::endl;
				return nullptr;
			} else if (attrTypeD.getString() == "p") {
				for (size_t j = 0; j < grp.anonymousCount(); ++j) {
					DL::Data attrValD = grp.at(j);
					if (attrValD.type() != DL::Data::T_Group) {
						PR_LOG(L_ERROR) << "Mesh position attribute is invalid." << std::endl;
						return nullptr;
					}

					bool ok;
					Eigen::Vector3f v = SceneLoader::getVector(attrValD.getGroup(), ok);

					if (ok) {
						for (int i = 0; i < 3; ++i)
							positionAttr[i].push_back(v[i]);
					} else {
						PR_LOG(L_ERROR) << "Mesh position attribute entry is invalid." << std::endl;
						return nullptr;
					}
				}
			} else if (attrTypeD.getString() == "n") {
				for (size_t j = 0; j < grp.anonymousCount(); ++j) {
					DL::Data attrValD = grp.at(j);
					if (attrValD.type() != DL::Data::T_Group) {
						PR_LOG(L_ERROR) << "Mesh normal attribute is invalid." << std::endl;
						return nullptr;
					}

					bool ok;
					Eigen::Vector3f v = SceneLoader::getVector(attrValD.getGroup(), ok);

					if (ok) {
						for (int i = 0; i < 3; ++i)
							normalAttr[i].push_back(v[i]);
					} else {
						PR_LOG(L_ERROR) << "Mesh normal attribute entry is invalid." << std::endl;
						return nullptr;
					}
				}
			} else if (attrTypeD.getString() == "t") {
				for (size_t j = 0; j < grp.anonymousCount(); ++j) {
					DL::Data attrValD = grp.at(j);
					if (attrValD.type() != DL::Data::T_Group) {
						PR_LOG(L_ERROR) << "Mesh texture attribute is invalid." << std::endl;
						return nullptr;
					}

					bool ok;
					Eigen::Vector3f v = SceneLoader::getVector(attrValD.getGroup(), ok);

					if (ok) {
						for (int i = 0; i < 2; ++i)
							uvAttr[i].push_back(v[i]);
					} else {
						PR_LOG(L_ERROR) << "Mesh texture attribute entry is invalid." << std::endl;
						return nullptr;
					}
				}
			} else if (attrTypeD.getString() == "dp") {
				PR_LOG(L_WARNING) << "Velocity attributes currently not supported." << std::endl;
			} else if (attrTypeD.getString() == "dn") {
				PR_LOG(L_WARNING) << "Derived normal attributes currently not supported." << std::endl;
			} else if (attrTypeD.getString() == "dt") {
				PR_LOG(L_WARNING) << "Derived texture attributes currently not supported." << std::endl;
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

	std::vector<uint32> materials;
	if (hasMaterials) {
		materials.reserve(materialsGrp.anonymousCount());
		for (size_t j = 0; j < materialsGrp.anonymousCount(); j++) {
			DL::Data indexD = materialsGrp.at(j);

			if (indexD.type() != DL::Data::T_Integer) {
				PR_LOG(L_ERROR) << "Given index is invalid." << std::endl;
				return nullptr;
			}

			auto index = indexD.getInt();

			if (index < 0) {
				PR_LOG(L_ERROR) << "Given index range is invalid." << std::endl;
				return nullptr;
			}

			materials.push_back(index);
		}
	}

	// Get indices (faces) -> only triangles!
	std::vector<uint32> faces[3];
	const size_t vertexCount = positionAttr[0].size();
	if (!hasFaces) { // Assume linear
		if ((vertexCount % 3) != 0) {
			PR_LOG(L_ERROR) << "Given position attribute count is not a multiply of 3." << std::endl;
			return nullptr;
		}

		if (hasMaterials && materialsGrp.anonymousCount() != vertexCount / 3) {
			PR_LOG(L_ERROR) << "Given material index count is not equal to face count." << std::endl;
			return nullptr;
		}

		for (int k = 0; k < 3; ++k)
			faces[k].reserve(vertexCount / 3);

		for (size_t j = 0; vertexCount; ++j) {
			faces[j % 3].push_back(j);
		}
	} else {
		if ((facesGrp.anonymousCount() % 3) != 0) {
			PR_LOG(L_ERROR) << "Given index face count is not a multiply of 3." << std::endl;
			return nullptr;
		}

		if (hasMaterials && materialsGrp.anonymousCount() != facesGrp.anonymousCount() / 3) {
			PR_LOG(L_ERROR) << "Given material index count is not equal to face count." << std::endl;
			return nullptr;
		}

		for (int k = 0; k < 3; ++k)
			faces[k].reserve(facesGrp.anonymousCount() / 3);

		for (size_t j = 0; j < facesGrp.anonymousCount(); ++j) {
			DL::Data iD = facesGrp.at(j);

			if (iD.type() != DL::Data::T_Integer) {
				PR_LOG(L_ERROR) << "Given index is invalid." << std::endl;
				return nullptr;
			}

			auto val = iD.getInt();

			if (val < 0 || (size_t)val >= vertexCount) {
				PR_LOG(L_ERROR) << "Given index range is invalid." << std::endl;
				return nullptr;
			}

			faces[j % 3].push_back(val);
		}
	}

	auto me = std::make_shared<TriMesh>();
	me->setVertices(positionAttr[0], positionAttr[1], positionAttr[2]);
	me->setNormals(normalAttr[0], normalAttr[1], normalAttr[2]);
	me->setUVs(uvAttr[0], uvAttr[1]);
	me->setMaterials(materials);
	me->setIndices(faces[0], faces[1], faces[2]);

	if (me->isValid()) {
		return me;
	} else {
		PR_LOG(L_ERROR) << "Loaded mesh is invalid." << std::endl;
		return nullptr;
	}
}
} // namespace PR
