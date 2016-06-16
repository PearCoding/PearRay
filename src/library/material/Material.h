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

		virtual Spectrum apply(const FacePoint& point, const PM::vec3& V, const PM::vec3& L) = 0;
		virtual Spectrum applyEmission(const FacePoint& point, const PM::vec3& V);
		
		/*
		 Calculate the PDF based on L. Can be infinitive to force predestined directions (e.g. glass)
		*/
		virtual float pdf(const FacePoint& point, const PM::vec3& V, const PM::vec3& L) = 0;

		/*
		 Sample a direction based on the uniform rnd value.
		*/
		virtual PM::vec3 sample(const FacePoint& point, const PM::vec3& rnd, const PM::vec3& V, float& pdf) = 0;

		inline virtual bool shouldIgnore(const Ray& in,const FacePoint& point);

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