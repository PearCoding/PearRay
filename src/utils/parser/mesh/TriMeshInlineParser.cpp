#include "TriMeshInlineParser.h"
#include "SceneLoader.h"
#include "Environment.h"
#include "Logger.h"

#include "geometry/Face.h"
#include "geometry/TriMesh.h"

// DataLisp
#include "DataLisp.h"
#include "DataContainer.h"
#include "DataGroup.h"
#include "DataArray.h"
#include "Data.h"
#include "SourceLogger.h"

using namespace PR;
namespace PRU
{
	TriMesh* TriMeshInlineParser::parse(SceneLoader* loader, Environment* env, DL::DataGroup* group) const
	{
		std::vector<PM::vec3> positionAttr;
		std::vector<PM::vec3> normalAttr;
		std::vector<PM::vec2> uvAttr;
		// TODO: More attributes!

		// First get attributes
		DL::DataGroup* facesGrp = nullptr;
		for(size_t i = 0; i < group->unnamedCount(); ++i)
		{
			DL::Data* d = group->at(i);
			if(d->isType() != DL::Data::T_Group)
			{
				PR_LOGGER.log(L_Error, M_Scene, "Invalid entry in mesh description.");
				return nullptr;
			}

			DL::DataGroup* grp = d->getGroup();
			if(grp->id() == "attribute")
			{
				DL::Data* attrTypeD = grp->getFromKey("type");
				if(!attrTypeD || attrTypeD->isType() != DL::Data::T_String)
				{
					PR_LOGGER.log(L_Error, M_Scene, "Mesh attribute has no valid type.");
					return nullptr;
				}
				else if(attrTypeD->getString() == "p")
				{
					for(size_t j = 0; j < grp->unnamedCount(); ++j)
					{
						DL::Data* attrValD = grp->at(j);
						if(attrValD->isType() != DL::Data::T_Array)
						{
							PR_LOGGER.log(L_Error, M_Scene, "Mesh position attribute is invalid.");
							return nullptr;
						}
						
						bool ok;
						PM::vec v = loader->getVector(attrValD->getArray(), ok);

						if(ok)
							positionAttr.push_back(v);
						else
						{
							PR_LOGGER.log(L_Error, M_Scene, "Mesh position attribute entry is invalid.");
							return nullptr;
						}
					}
				}
				else if(attrTypeD->getString() == "n")
				{
					for(size_t j = 0; j < grp->unnamedCount(); ++j)
					{
						DL::Data* attrValD = grp->at(j);
						if(attrValD->isType() != DL::Data::T_Array)
						{
							PR_LOGGER.log(L_Error, M_Scene, "Mesh normal attribute is invalid.");
							return nullptr;
						}
						
						bool ok;
						PM::vec v = loader->getVector(attrValD->getArray(), ok);

						if(ok)
							normalAttr.push_back(PM::pm_SetW(v, 0.0f));
						else
						{
							PR_LOGGER.log(L_Error, M_Scene, "Mesh normal attribute entry is invalid.");
							return nullptr;
						}
					}
				}
				else if(attrTypeD->getString() == "t")
				{
					for(size_t j = 0; j < grp->unnamedCount(); ++j)
					{
						DL::Data* attrValD = grp->at(j);
						if(attrValD->isType() != DL::Data::T_Array)
						{
							PR_LOGGER.log(L_Error, M_Scene, "Mesh texture attribute is invalid.");
							return nullptr;
						}
						
						bool ok;
						PM::vec v = loader->getVector(attrValD->getArray(), ok);

						if(ok)
							uvAttr.push_back(PM::pm_SetZ(PM::pm_SetW(v, 0.0f), 0.0f));
						else
						{
							PR_LOGGER.log(L_Error, M_Scene, "Mesh texture attribute entry is invalid.");
							return nullptr;
						}
					}
				}
				else if(attrTypeD->getString() == "dp")
				{
					PR_LOGGER.log(L_Warning, M_Scene, "Velocity attributes currently not supported.");
				}
				else if(attrTypeD->getString() == "dn")
				{
					PR_LOGGER.log(L_Warning, M_Scene, "Derived normal attributes currently not supported.");
				}
				else if(attrTypeD->getString() == "dt")
				{
					PR_LOGGER.log(L_Warning, M_Scene, "Derived texture attributes currently not supported.");
				}
				else if(attrTypeD->getString() == "u")
				{
					PR_LOGGER.log(L_Warning, M_Scene, "User attributes currently not supported.");
				}
				else
				{
					PR_LOGGER.log(L_Error, M_Scene, "Unknown mesh attribute.");
					return nullptr;
				}
			}
			else if(grp->id() == "faces")
			{
				if(facesGrp)
					PR_LOGGER.log(L_Warning, M_Scene, "Faces already set for mesh.");
				else
					facesGrp = grp;
			}
			else
			{
				PR_LOGGER.log(L_Error, M_Scene, "Invalid entry in mesh description.");
				return nullptr;
			}
		}

		// Check validity
		if(positionAttr.empty())
		{
			PR_LOGGER.log(L_Error, M_Scene, "No position attribute given.");
			return nullptr;
		}

		if(!normalAttr.empty() && normalAttr.size() != positionAttr.size())
		{
			PR_LOGGER.log(L_Error, M_Scene, "Normal attribute does not match position attribute in size.");
			return nullptr;
		}

		if(!uvAttr.empty() && uvAttr.size() != positionAttr.size())
		{
			PR_LOGGER.log(L_Error, M_Scene, "Texture attribute does not match position attribute in size.");
			return nullptr;
		}

		// Get indices (faces) -> only triangles!
		std::vector<Face*> faces;
		if(!facesGrp)// Assume linear
		{
			if((positionAttr.size() % 3) != 0)
			{
				PR_LOGGER.log(L_Error, M_Scene, "Given position attribute count is not a multiply of 3.");
				return nullptr;
			}

			faces.reserve(positionAttr.size()/3);
			for(size_t j = 0; positionAttr.size(); j += 3)
			{
				Face* face = new Face();
				face->V[0] = positionAttr[j];
				face->V[1] = positionAttr[j+1];
				face->V[2] = positionAttr[j+2];

				if(!normalAttr.empty())
				{
					face->N[0] = normalAttr[j];
					face->N[1] = normalAttr[j+1];
					face->N[2] = normalAttr[j+2];
				}

				if(!uvAttr.empty())
				{
					face->UV[0] = uvAttr[j];
					face->UV[1] = uvAttr[j+1];
					face->UV[2] = uvAttr[j+2];
				}
				else
				{
					face->UV[0] = PM::pm_Zero();
					face->UV[1] = PM::pm_Zero();
					face->UV[2] = PM::pm_Zero();
				}

				faces.push_back(face);
			}
		}
		else
		{
			if((facesGrp->unnamedCount() % 3) != 0)
			{
				PR_LOGGER.log(L_Error, M_Scene, "Given index face count is not a multiply of 3.");
				return nullptr;
			}

			faces.reserve(facesGrp->unnamedCount()/3);
			for(size_t j = 0; j < facesGrp->unnamedCount(); j += 3)
			{
				DL::Data* i1D = facesGrp->at(j);
				DL::Data* i2D = facesGrp->at(j+1);
				DL::Data* i3D = facesGrp->at(j+2);

				if(	i1D->isType() != DL::Data::T_Integer ||
					i2D->isType() != DL::Data::T_Integer ||
					i3D->isType() != DL::Data::T_Integer)
				{
					PR_LOGGER.log(L_Error, M_Scene, "Given index is invalid.");
					return nullptr;
				}

				auto i1 = i1D->getInt(); 
				auto i2 = i2D->getInt(); 
				auto i3 = i3D->getInt();

				if (i1 < 0 || (size_t)i1 >= positionAttr.size() ||
					i2 < 0 || (size_t)i2 >= positionAttr.size() ||
					i3 < 0 || (size_t)i3 >= positionAttr.size()) 
				{
					PR_LOGGER.log(L_Error, M_Scene, "Given index range is invalid.");
					return nullptr;
				}

				Face* face = new Face();
				face->V[0] = positionAttr[i1];
				face->V[1] = positionAttr[i2];
				face->V[2] = positionAttr[i3];

				if(!normalAttr.empty())
				{
					face->N[0] = normalAttr[i1];
					face->N[1] = normalAttr[i2];
					face->N[2] = normalAttr[i3];
				}

				if(!uvAttr.empty())
				{
					face->UV[0] = uvAttr[i1];
					face->UV[1] = uvAttr[i2];
					face->UV[2] = uvAttr[i3];
				}
				else
				{
					face->UV[0] = PM::pm_Zero();
					face->UV[1] = PM::pm_Zero();
					face->UV[2] = PM::pm_Zero();
				}

				faces.push_back(face);
			}
		}

		TriMesh* me = new TriMesh();
		me->setFaces(faces);

		if(normalAttr.empty())
		{
			PR_LOGGER.log(L_Warning, M_Scene, "No normals given for mesh. Calculating it instead.");
			me->calcNormals();
		}
		
		PR_LOGGER.log(L_Info, M_Scene, "Mesh KDTree:");
		me->build();
		return me;
	}
}