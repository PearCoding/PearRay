#pragma once

#include "texture/Texture2D.h"

namespace PR
{
	class RenderEntity;
	class FacePoint;
	class Ray;
	class Renderer;
	class PR_LIB Material
	{
	public:
		Material();

		virtual Spectrum apply(const FacePoint& point, const PM::vec3& V, const PM::vec3& L, const Spectrum& Li) = 0;
		virtual Spectrum applyEmission(const FacePoint& point, const PM::vec3& V);

		// Parameter 'dir' is an output!
		// Return value is weight of vector
		virtual float emitReflectionVector(const FacePoint& point, const PM::vec3& V, PM::vec3& dir) = 0;
		virtual float emitTransmissionVector(const FacePoint& point, const PM::vec3& V, PM::vec3& dir) = 0;

		virtual bool shouldIgnore_Simple(const Ray& in, RenderEntity* entity);

		inline virtual bool shouldIgnore_Complex(const Ray& in, RenderEntity* entity, const FacePoint& point)
		{
			return false;
		}

		virtual float roughness(const FacePoint& point) const = 0;// How much specular!

		bool isLight() const;

		Texture2D* emission() const;
		void setEmission(Texture2D* spec);

		bool canBeShaded() const;
		void enableShading(bool b);

		void enableSelfShadow(bool b);
		bool canBeSelfShadowed() const;

		void enableCameraVisibility(bool b);
		bool isCameraVisible() const;

	private:
		Texture2D* mEmission;

		bool mIsLight;
		bool mCanBeShaded;
		bool mSelfShadow;
		bool mCameraVisible;
	};
}