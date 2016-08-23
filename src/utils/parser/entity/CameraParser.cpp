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
	Entity* CameraParser::parse(SceneLoader* loader, Environment* env, const std::string& name, Entity* parent,
		const std::string& obj, DL::DataGroup* group) const
	{
		DL::Data* projectionD = group->getFromKey("projection");
		DL::Data* fovHD = group->getFromKey("fovH");
		DL::Data* fovVD = group->getFromKey("fovV");
		DL::Data* widthD = group->getFromKey("width");
		DL::Data* heightD = group->getFromKey("height");
		DL::Data* lookAtD = group->getFromKey("lookAt");
		DL::Data* fstopD = group->getFromKey("fstop");
		DL::Data* apertureRadiusD = group->getFromKey("apertureRadius");
		DL::Data* zoomD = group->getFromKey("zoom");

		StandardCamera* camera = new StandardCamera(name, parent);

		if (fovVD || fovHD)
		{
			float fovH = 60;
			float fovV = 45;

			if (fovHD && fovHD->isNumber())
			{
				fovH = fovHD->getFloatConverted();
			}

			if (fovVD && fovVD->isNumber())
			{
				fovV = fovVD->getFloatConverted();
			}

			camera->setWithAngle(PM::pm_DegToRad(fovH), PM::pm_DegToRad(fovV));
		}
		else
		{
			float width = 1;
			float height = 1;

			if (widthD && widthD->isNumber())
			{
				width = widthD->getFloatConverted();
			}

			if (heightD && heightD->isNumber())
			{
				height = heightD->getFloatConverted();
			}

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

		if (lookAtD && lookAtD->isType() == DL::Data::T_Array)
		{
			bool ok;
			PM::vec3 look = loader->getVector(lookAtD->getArray(), ok);

			if (ok)
			{
				camera->lookAt(look);
			}
		}

		if (projectionD && projectionD->isType() == DL::Data::T_String)
		{
			std::string proj = projectionD->getString();
			std::transform(proj.begin(), proj.end(), proj.begin(), ::tolower);

			if (proj == "orthographic" || proj == "ortho")
			{
				camera->setOrthographic(true);
			}
		}

		if (fstopD && fstopD->isNumber())
		{
			camera->setFStop(fstopD->getFloatConverted());
		}

		if (apertureRadiusD && apertureRadiusD->isNumber())
		{
			camera->setApertureRadius(apertureRadiusD->getFloatConverted());
		}

		return camera;
	}
}