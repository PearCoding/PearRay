#include "CameraParser.h"
#include "SceneLoader.h"
#include "Environment.h"
#include "Logger.h"

#include "camera/OrthographicCamera.h"
#include "camera/PerspectiveCamera.h"

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
	Entity* CameraParser::parse(SceneLoader* loader, Environment* env, const std::string& name, Entity* parent,
		const std::string& obj, DL::DataGroup* group) const
	{
		DL::Data* projectionD = group->getFromKey("projection");

		if (projectionD && projectionD->isType() == DL::Data::T_String)
		{
			if (projectionD->getString() == "perspective")
			{
				DL::Data* fovHD = group->getFromKey("fovH");
				DL::Data* fovVD = group->getFromKey("fovV");
				DL::Data* widthD = group->getFromKey("width");
				DL::Data* heightD = group->getFromKey("height");
				DL::Data* lensDistD = group->getFromKey("lensDistance");
				DL::Data* lookAtD = group->getFromKey("lookAt");

				PerspectiveCamera* camera = new PerspectiveCamera(name, parent);

				if (fovVD || fovHD)
				{
					float fovH = 60;
					float fovV = 45;
					float lensDist = 0.1f;

					if (fovHD && fovHD->isNumber())
					{
						fovH = fovHD->getFloatConverted();
					}

					if (fovVD && fovVD->isNumber())
					{
						fovV = fovVD->getFloatConverted();
					}

					if (lensDistD && lensDistD->isNumber())
					{
						lensDist = lensDistD->getFloatConverted();
					}

					camera->setWithAngle(PM::pm_DegToRad(fovH), PM::pm_DegToRad(fovV), lensDist);
				}
				else
				{
					float width = 1;
					float height = 1;
					float lensDist = 0.1f;

					if (widthD && widthD->isNumber())
					{
						width = widthD->getFloatConverted();
					}

					if (heightD && heightD->isNumber())
					{
						height = heightD->getFloatConverted();
					}

					if (lensDistD && lensDistD->isNumber())
					{
						lensDist = lensDistD->getFloatConverted();
					}

					camera->setWithSize(width, height, lensDist);
				}

				if (lookAtD && lookAtD->isType() == DL::Data::T_Array)
				{
					bool ok;
					PM::vec3 look = loader->getVector(lookAtD->getArray(), ok);

					if (ok)
					{
						camera->lookAt(look);
					}
				}
				return camera;
			}
			else if (projectionD->getString() == "orthographic")
			{
				DL::Data* fovHD = group->getFromKey("fovH");
				DL::Data* fovVD = group->getFromKey("fovV");
				DL::Data* widthD = group->getFromKey("width");
				DL::Data* heightD = group->getFromKey("height");
				DL::Data* lensDistD = group->getFromKey("lensDistance");
				DL::Data* lookAtD = group->getFromKey("lookAt");

				OrthographicCamera* camera = new OrthographicCamera(name, parent);

				if (fovVD || fovHD)
				{
					float fovH = 60;
					float fovV = 45;
					float lensDist = 0.1f;

					if (fovHD && fovHD->isNumber())
					{
						fovH = fovHD->getFloatConverted();
					}

					if (fovVD && fovVD->isNumber())
					{
						fovV = fovVD->getFloatConverted();
					}

					if (lensDistD && lensDistD->isNumber())
					{
						lensDist = lensDistD->getFloatConverted();
					}

					camera->setWithAngle(PM::pm_DegToRad(fovH), PM::pm_DegToRad(fovV), lensDist);
				}
				else
				{
					float width = 1;
					float height = 1;
					float lensDist = 0.1f;

					if (widthD && widthD->isNumber())
					{
						width = widthD->getFloatConverted();
					}

					if (heightD && heightD->isNumber())
					{
						height = heightD->getFloatConverted();
					}

					if (lensDistD && lensDistD->isNumber())
					{
						lensDist = lensDistD->getFloatConverted();
					}

					camera->setWithSize(width, height, lensDist);
				}

				if (lookAtD && lookAtD->isType() == DL::Data::T_Array)
				{
					bool ok;
					PM::vec3 look = loader->getVector(lookAtD->getArray(), ok);

					if (ok)
					{
						camera->lookAt(look);
					}
				}

				return camera;
			}
			else
			{
				PR_LOGGER.logf(L_Error, M_Scene, "Unknown camera projection for entity %s given.", name.c_str());
				return nullptr;
			}
		}
		else
		{
			PR_LOGGER.logf(L_Error, M_Scene, "No valid camera projection for entity %s given.", name.c_str());
			return nullptr;
		}
	}
}