#include "CameraParser.h"
#include "SceneLoader.h"
#include "Environment.h"
#include "Logger.h"

#include "camera/StandardCamera.h"

// DataLisp
#include "DataLisp.h"
#include "DataContainer.h"
#include "DataGroup.h"
#include "DataArray.h"
#include "Data.h"
#include "SourceLogger.h"

#include <algorithm>

using namespace PR;
namespace PRU
{
	Entity* CameraParser::parse(SceneLoader* loader, Environment* env, const std::string& name,
		const std::string& obj, DL::DataGroup* group) const
	{
		DL::Data* projectionD = group->getFromKey("projection");
		DL::Data* fovHD = group->getFromKey("fovH");
		DL::Data* fovVD = group->getFromKey("fovV");
		DL::Data* widthD = group->getFromKey("width");
		DL::Data* heightD = group->getFromKey("height");
		DL::Data* fstopD = group->getFromKey("fstop");
		DL::Data* apertureRadiusD = group->getFromKey("apertureRadius");
		DL::Data* zoomD = group->getFromKey("zoom");

		DL::Data* localDirD = group->getFromKey("localDirection");
		DL::Data* localRightD = group->getFromKey("localRight");
		DL::Data* localUpD = group->getFromKey("localUp");

		StandardCamera* camera = new StandardCamera(name);

		if (fovVD || fovHD)
		{
			float fovH = 60;
			float fovV = 45;

			if (fovHD && fovHD->isNumber())
				fovH = fovHD->getFloatConverted();

			if (fovVD && fovVD->isNumber())
				fovV = fovVD->getFloatConverted();

			camera->setWithAngle(PM::pm_DegToRad(fovH), PM::pm_DegToRad(fovV));
		}
		else
		{
			float width = 1;
			float height = 1;

			if (widthD && widthD->isNumber())
				width = widthD->getFloatConverted();

			if (heightD && heightD->isNumber())
				height = heightD->getFloatConverted();

			camera->setWithSize(width, height);
		}

		if (zoomD && zoomD->isNumber())
		{
			float zoom = zoomD->getFloatConverted();

			if (zoom > PM_EPSILON)
			{
				camera->setWidth(camera->width() / zoom);
				camera->setHeight(camera->height() / zoom);
			}
		}

		if (projectionD && projectionD->isType() == DL::Data::T_String)
		{
			std::string proj = projectionD->getString();
			std::transform(proj.begin(), proj.end(), proj.begin(), ::tolower);

			if (proj == "orthographic" || proj == "orthogonal" || proj == "ortho")
				camera->setOrthographic(true);
		}

		if (fstopD && fstopD->isNumber())
		{
			camera->setFStop(fstopD->getFloatConverted());
		}

		if (apertureRadiusD && apertureRadiusD->isNumber())
		{
			camera->setApertureRadius(apertureRadiusD->getFloatConverted());
		}

		if (localDirD && localDirD->isType() == DL::Data::T_Array)
		{
			bool ok;
			auto v = loader->getVector(localDirD->getArray(), ok);
			if(ok)
				camera->setLocalDirection(v);
			else
				PR_LOGGER.logf(L_Warning, M_Scene, "Invalid local direction given.");
		}

		if (localRightD && localRightD->isType() == DL::Data::T_Array)
		{
			bool ok;
			auto v = loader->getVector(localRightD->getArray(), ok);
			if(ok)
				camera->setLocalRight(v);
			else
				PR_LOGGER.logf(L_Warning, M_Scene, "Invalid local right given.");
		}

		if (localUpD && localUpD->isType() == DL::Data::T_Array)
		{
			bool ok;
			auto v = loader->getVector(localUpD->getArray(), ok);
			if(ok)
				camera->setLocalUp(v);
			else
				PR_LOGGER.logf(L_Warning, M_Scene, "Invalid local up given.");
		}

		return camera;
	}
}