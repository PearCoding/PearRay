#include "CameraParser.h"
#include "SceneLoader.h"
#include "Environment.h"
#include "Logger.h"

#include "camera/StandardCamera.h"

#include "DataLisp.h"

#include <algorithm>

using namespace PR;
namespace PRU
{
	Entity* CameraParser::parse(SceneLoader* loader, Environment* env, const std::string& name,
		const std::string& obj, const DL::DataGroup& group) const
	{
		DL::Data projectionD = group.getFromKey("projection");
		DL::Data fovHD = group.getFromKey("fovH");
		DL::Data fovVD = group.getFromKey("fovV");
		DL::Data widthD = group.getFromKey("width");
		DL::Data heightD = group.getFromKey("height");
		DL::Data fstopD = group.getFromKey("fstop");
		DL::Data apertureRadiusD = group.getFromKey("apertureRadius");
		DL::Data zoomD = group.getFromKey("zoom");

		DL::Data localDirD = group.getFromKey("localDirection");
		DL::Data localRightD = group.getFromKey("localRight");
		DL::Data localUpD = group.getFromKey("localUp");

		StandardCamera* camera = new StandardCamera(env->scene()->entities().size()+1, name);

		if (fovVD.isNumber() || fovHD.isNumber())
		{
			float fovH = 60;
			float fovV = 45;

			if (fovHD.isNumber())
				fovH = fovHD.getNumber();

			if (fovVD.isNumber())
				fovV = fovVD.getNumber();

			camera->setWithAngle(PM::pm_DegToRad(fovH), PM::pm_DegToRad(fovV));
		}
		else
		{
			float width = 1;
			float height = 1;

			if (widthD.isNumber())
				width = widthD.getNumber();

			if (heightD.isNumber())
				height = heightD.getNumber();

			camera->setWithSize(width, height);
		}

		if (zoomD.isNumber())
		{
			float zoom = zoomD.getNumber();

			if (zoom > PM_EPSILON)
			{
				camera->setWidth(camera->width() / zoom);
				camera->setHeight(camera->height() / zoom);
			}
		}

		if (projectionD.type() == DL::Data::T_String)
		{
			std::string proj = projectionD.getString();
			std::transform(proj.begin(), proj.end(), proj.begin(), ::tolower);

			if (proj == "orthographic" || proj == "orthogonal" || proj == "ortho")
				camera->setOrthographic(true);
		}

		if (fstopD.isNumber())
			camera->setFStop(fstopD.getNumber());

		if (apertureRadiusD.isNumber())
			camera->setApertureRadius(apertureRadiusD.getNumber());

		if (localDirD.type() == DL::Data::T_Group)
		{
			bool ok;
			auto v = loader->getVector(localDirD.getGroup(), ok);
			if(ok)
				camera->setLocalDirection(v);
			else
				PR_LOGGER.logf(L_Warning, M_Scene, "Invalid local direction given.");
		}

		if (localRightD.type() == DL::Data::T_Group)
		{
			bool ok;
			auto v = loader->getVector(localRightD.getGroup(), ok);
			if(ok)
				camera->setLocalRight(v);
			else
				PR_LOGGER.logf(L_Warning, M_Scene, "Invalid local right given.");
		}

		if (localUpD.type() == DL::Data::T_Group)
		{
			bool ok;
			auto v = loader->getVector(localUpD.getGroup(), ok);
			if(ok)
				camera->setLocalUp(v);
			else
				PR_LOGGER.logf(L_Warning, M_Scene, "Invalid local up given.");
		}

		return camera;
	}
}