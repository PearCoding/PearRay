#include "GSLMaterial.h"
#include "ray/Ray.h"
#include "geometry/FacePoint.h"
#include "renderer/Renderer.h"
#include "entity/RenderEntity.h"

using namespace PR;

namespace PRU
{
	GSLMaterial::GSLMaterial() :
		Material(), mCanBeShaded(true),
		mLight(false), mSelfShadow(true), mCameraVisible(true)
	{
	}

	void GSLMaterial::enableLight(bool b)
	{
		mLight = b;
	}

	bool GSLMaterial::isLight() const
	{
		return mLight;
	}

	bool GSLMaterial::canBeShaded() const
	{
		return mCanBeShaded;
	}

	void GSLMaterial::enableShading(bool b)
	{
		mCanBeShaded = b;
	}

	void GSLMaterial::enableSelfShadow(bool b)
	{
		mSelfShadow = b;
	}

	bool GSLMaterial::canBeSelfShadowed() const
	{
		return mSelfShadow;
	}

	void GSLMaterial::enableCameraVisibility(bool b)
	{
		mCameraVisible = b;
	}

	bool GSLMaterial::isCameraVisible() const
	{
		return mCameraVisible;
	}

	void GSLMaterial::apply(Ray& in, RenderEntity* entity, const FacePoint& point, Renderer* renderer)
	{
	}
}