#pragma once

#include "shader/ShaderOutput.h"

namespace PR
{
	class RenderEntity;
	struct ShaderClosure;
	class Ray;
	class Renderer;
	class PR_LIB Material
	{
	public:
		Material();
		virtual ~Material() {}

		virtual Spectrum apply(const ShaderClosure& point, const PM::vec3& L) = 0;
		
		/*
		 Calculate the PDF based on L. Can be infinitive to force predestined directions (e.g. glass)
		*/
		virtual float pdf(const ShaderClosure& point, const PM::vec3& L) = 0;

		/*
		 Sample a direction based on the uniform rnd value.
		*/
		virtual PM::vec3 sample(const ShaderClosure& point, const PM::vec3& rnd, float& pdf) = 0;

		bool isLight() const;

		SpectralShaderOutput* emission() const;
		void setEmission(SpectralShaderOutput* spec);

		bool canBeShaded() const;
		void enableShading(bool b);

		void enableSelfShadow(bool b);
		bool canBeSelfShadowed() const;

		void enableCameraVisibility(bool b);
		bool isCameraVisible() const;

	private:
		SpectralShaderOutput* mEmission;

		bool mIsLight;
		bool mCanBeShaded;
		bool mSelfShadow;
		bool mCameraVisible;
	};
}