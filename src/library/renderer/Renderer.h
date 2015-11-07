#pragma once

#include "RenderResult.h"
#include "spectral/Spectrum.h"

#include <list>
#include <mutex>

namespace PR
{
	class Camera;
	class FacePoint;
	class GeometryEntity;
	class Scene;
	class Ray;
	class RenderThread;
	class PR_LIB Renderer
	{
	public:
		Renderer(uint32 width, uint32 height, Camera* cam, Scene* scene);
		virtual ~Renderer();

		void setWidth(uint32 w);
		uint32 width() const;

		void setHeight(uint32 h);
		uint32 height() const;
		
		void render(uint32 threads = 0);
		void render(uint32 x, uint32 y);

		size_t pixelsRendered() const;

		GeometryEntity* shoot(Ray& ray, FacePoint& collisionPoint);

		bool isFinished();
		void waitForFinish();
		void stop();

		RenderResult& result();

		size_t rayCount() const;
	private:
		uint32 mWidth;
		uint32 mHeight;

		Camera* mCamera;
		Scene* mScene;
		RenderResult mResult;

		std::list<RenderThread*> mThreads;

		Spectrum mIdentitySpectrum;

		std::mutex mStatisticMutex;
		size_t mPixelsRendered;
		size_t mRayCount;
	};
}