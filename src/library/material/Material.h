#pragma once

#include "shader/ShaderOutput.h"

namespace PR
{
	class RenderEntity;
	struct ShaderClosure;
	class Ray;
	class RenderContext;
	class PR_LIB Material
	{
	public:
		Material(uint32 id);
		virtual ~Material() {}

		/*
		 Evaluate the BxDF based on L and point information.
		*/
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
		virtual PM::vec3 samplePath(const ShaderClosure& point, const PM::vec3& rnd, float& pdf, Spectrum& path_weight, uint32 path)
		{
			path_weight.fill(1);
			return sample(point, rnd, pdf);
		}

		/*
		 Returns amount of special paths in the material. (like reflection/refraction)
		 */
		virtual uint32 samplePathCount() const
		{
			return 1;
		}

		inline uint32 id() const
		{
			return mID;
		}

		bool isLight() const { return mEmission != nullptr; }

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

		virtual void setup(RenderContext* context) {};

		virtual std::string dumpInformation() const;
	private:
		SpectralShaderOutput* mEmission;

		const uint32 mID;
		bool mCanBeShaded;
		bool mShadow;
		bool mSelfShadow;
		bool mCameraVisible;
	};
}
