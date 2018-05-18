#include "TriMeshInlineParser.h"
#include "Environment.h"
#include "Logger.h"
#include "SceneLoader.h"

#include "geometry/Face.h"
#include "geometry/TriMesh.h"

#include "DataLisp.h"

namespace PR {
std::shared_ptr<TriMesh> TriMeshInlineParser::parse(Environment* env, const DL::DataGroup& group) const
{
	std::vector<Vector3f> positionAttr;
	std::vector<Vector3f> normalAttr;
	std::vector<Vector2f> uvAttr;
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
						positionAttr.push_back(Vector3f{ v[0], v[1], v[2] });
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
						normalAttr.push_back(Vector3f{ v[0], v[1], v[2] });
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
						uvAttr.push_back(Vector2f{ v[0], v[1] });
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
	if (positionAttr.empty()) {
		PR_LOG(L_ERROR) << "No position attribute given." << std::endl;
		return nullptr;
	}

	if (normalAttr.empty() || normalAttr.size() != positionAttr.size()) {
		PR_LOG(L_ERROR) << "Normal attribute does not match position attribute in size or is empty." << std::endl;
		return nullptr;
	}

	if (!uvAttr.empty() && uvAttr.size() != positionAttr.size()) {
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
	std::vector<Vector3u64> faces;
	if (!hasFaces) { // Assume linear
		if ((positionAttr.size() % 3) != 0) {
			PR_LOG(L_ERROR) << "Given position attribute count is not a multiply of 3." << std::endl;
			return nullptr;
		}

		if (hasMaterials && materialsGrp.anonymousCount() != positionAttr.size() / 3) {
			PR_LOG(L_ERROR) << "Given material index count is not equal to face count." << std::endl;
			return nullptr;
		}

		faces.reserve(positionAttr.size() / 3);
		for (size_t j = 0; positionAttr.size(); j += 3) {
			faces.push_back(Vector3u64{ j + 0, j + 1, j + 2 });
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

		faces.reserve(facesGrp.anonymousCount() / 3);
		for (size_t j = 0; j < facesGrp.anonymousCount(); j += 3) {
			DL::Data i1D = facesGrp.at(j);
			DL::Data i2D = facesGrp.at(j + 1);
			DL::Data i3D = facesGrp.at(j + 2);

			if (i1D.type() != DL::Data::T_Integer || i2D.type() != DL::Data::T_Integer || i3D.type() != DL::Data::T_Integer) {
				PR_LOG(L_ERROR) << "Given index is invalid." << std::endl;
				return nullptr;
			}

			auto i1 = i1D.getInt();
			auto i2 = i2D.getInt();
			auto i3 = i3D.getInt();

			if (i1 < 0 || (size_t)i1 >= positionAttr.size() || i2 < 0 || (size_t)i2 >= positionAttr.size() || i3 < 0 || (size_t)i3 >= positionAttr.size()) {
				PR_LOG(L_ERROR) << "Given index range is invalid." << std::endl;
				return nullptr;
			}

			faces.push_back(Vector3u64{ static_cast<uint64>(i1), static_cast<uint64>(i2), static_cast<uint64>(i3) });
		}
	}

	auto me = std::make_shared<TriMesh>();
	me->setVertices(positionAttr);
	me->setNormals(normalAttr);
	me->setUVs(uvAttr);
	me->setMaterials(materials);
	me->setIndices(faces);

	if (me->isValid()) {
		me->build();
		return me;
	} else {
		PR_LOG(L_ERROR) << "Loaded mesh is invalid." << std::endl;
		return nullptr;
	}
}
} // namespace PR
