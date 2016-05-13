#pragma once

#include "Integrator.h"

namespace PR
{
	class PR_LIB BiDirectIntegrator : public Integrator
	{
	public:
		BiDirectIntegrator();
		~BiDirectIntegrator();

		void init(Renderer* renderer) override;
		Spectrum apply(Ray& in, RenderEntity* entity, const FacePoint& point, RenderContext* context) override;

	private:
		static bool handleObject(Ray& in, RenderEntity* entity, const FacePoint& point, Renderer* renderer, bool applyMat, bool& store);

		struct ThreadData
		{
			Ray* CameraPath;
			RenderEntity** CameraEntities;
			FacePoint* CameraFacePoints;
			bool* CameraDiff;

			Ray* LightPath;// For every light a path (LightCount * MaxLightSamples * MaxPathCount)
			//RenderEntity** LightEntities;
			uint32* LightMaxDepth;
		};
		ThreadData* mThreadData;
		uint32 mThreadCount;
	};
}