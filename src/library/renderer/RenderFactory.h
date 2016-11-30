#pragma once

#include "RenderSettings.h"

#include <string>

namespace PR
{
	class Camera;
	class GPU;
	class RenderContext;
	class Scene;
	class PR_LIB RenderFactory
	{
	public:
		RenderFactory(uint32 width, uint32 height, Camera* cam, Scene* scene,
			const std::string& workingDir, bool useGPU = true);
		virtual ~RenderFactory();

		inline void setFullWidth(uint32 w) { mFullWidth = w; }
		inline uint32 fullWidth() const { return mFullWidth; }

		inline void setFullHeight(uint32 h) { mFullHeight = h; }
		inline uint32 fullHeight() const { return mFullHeight; }

		uint32 cropWidth() const;
		uint32 cropHeight() const;
		uint32 cropOffsetX() const;
		uint32 cropOffsetY() const;

		// index = image index; should be less then itx*ity!
		// itx = image tile count x
		// ity = image tile count y
		RenderContext* create(uint32 index, uint32 itx, uint32 ity) const;
		inline RenderContext* create() const { return create(0,1,1); }

		inline void setSettings(const RenderSettings& s) { mRenderSettings = s; }
		inline RenderSettings& settings() { return mRenderSettings; }
		inline const RenderSettings& settings() const { return mRenderSettings; }

		inline void setScene(Scene* scene) { mScene = scene; }
		inline Scene* scene() const { return mScene; }

		inline void setCamera(Camera* camera) { mCamera = camera; }
		inline Camera* camera() const { return mCamera; }

		inline void setWorkingDir(const std::string& dir) { mWorkingDir = dir; }
		inline std::string workingDir() const { return mWorkingDir; }

		inline GPU* gpu() const { return mGPU; }
	private:
		uint32 mFullWidth;
		uint32 mFullHeight;
		std::string mWorkingDir;

		Camera* mCamera;
		Scene* mScene;

		GPU* mGPU;

		RenderSettings mRenderSettings;
	};
}