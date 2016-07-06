#pragma once

#include "Material.h"
#include "texture/Texture2D.h"

namespace PR
{
	class PR_LIB WardMaterial : public Material
	{
	public:
		WardMaterial();

		Texture2D* albedo() const;
		void setAlbedo(Texture2D* diffSpec);

		Texture2D* specularity() const;
		void setSpecularity(Texture2D* spec);

		Data2D* roughnessX() const;
		void setRoughnessX(Data2D* data);

		Data2D* roughnessY() const;
		void setRoughnessY(Data2D* data);

		Spectrum apply(const FacePoint& point, const PM::vec3& V, const PM::vec3& L) override;
		float pdf(const FacePoint& point, const PM::vec3& V, const PM::vec3& L) override;
		PM::vec3 sample(const FacePoint& point, const PM::vec3& rnd, const PM::vec3& V, float& pdf) override;

	private:
		Texture2D* mAlbedo;
		Texture2D* mSpecularity;
		Data2D* mRoughnessX;
		Data2D* mRoughnessY;
	};
}