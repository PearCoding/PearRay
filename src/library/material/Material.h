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

		virtual Spectrum eval(const ShaderClosure& point, const PM::vec3& L, float NdotL) = 0;
		
		/*
		 Calculate the PDF based on L. Can be infinitive to force predestined directions (e.g. glass)
		*/
		virtual float pdf(const ShaderClosure& point, const PM::vec3& L, float NdotL) = 0;

		/*
		 Sample a direction based on the uniform rnd value. (Roussian Roulette)
		*/
		virtual PM::vec3 sample(const ShaderClosure& point, const PM::vec3& rnd, float& pdf) = 0;

		/*
		 Sample a direction based on the uniform rnd value. (Non roussian roulette)
		*/
		virtual PM::vec3 samplePath(const ShaderClosure& point, const PM::vec3& rnd, float& pdf, float& weight, uint32 path)
		{
			weight = 1;
			return sample(point, rnd, pdf);
		}

		/*
		 Returns amount of special paths in the material. (like reflection/refraction)
		 */
		virtual uint32 samplePathCount() const
		{
			return 1;
		}

		bool isLight() const;

		SpectralShaderOutput* emission() const;
		void setEmission(SpectralShaderOutput* spec);

		bool canBeShaded() const;
		void enableShading(bool b);

		void enableShadow(bool b);
		bool allowsShadow() const;

		void enableSelfShadow(bool b);
		bool allowsSelfShadow() const;

		void enableCameraVisibility(bool b);
		bool isCameraVisible() const;

	private:
		SpectralShaderOutput* mEmission;

		bool mIsLight;
		bool mCanBeShaded;
		bool mShadow;
		bool mSelfShadow;
		bool mCameraVisible;
	};
}