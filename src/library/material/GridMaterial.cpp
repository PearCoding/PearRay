#include "GridMaterial.h"
#include "shader/ShaderClosure.h"

#include <sstream>

namespace PR {
GridMaterial::GridMaterial(uint32 id)
	: Material(id)
	, mFirst(nullptr)
	, mSecond(nullptr)
	, mGridCount(10)
	, mTiledUV(true)
{
}

void GridMaterial::setFirstMaterial(const std::shared_ptr<Material>& mat)
{
	PR_ASSERT(mat.get() != this, "Self assignment is invalid!");
	mFirst = mat;
}

const std::shared_ptr<Material>& GridMaterial::firstMaterial() const
{
	return mFirst;
}

void GridMaterial::setSecondMaterial(const std::shared_ptr<Material>& mat)
{
	PR_ASSERT(mat.get() != this, "Self assignment is invalid!");
	mSecond = mat;
}

const std::shared_ptr<Material>& GridMaterial::secondMaterial() const
{
	return mSecond;
}

void GridMaterial::setGridCount(int i)
{
	mGridCount = i;
}

int GridMaterial::gridCount() const
{
	return mGridCount;
}

void GridMaterial::setTileUV(bool b)
{
	mTiledUV = b;
}

bool GridMaterial::tileUV() const
{
	return mTiledUV;
}

Spectrum GridMaterial::eval(const ShaderClosure& point, const Eigen::Vector3f& L, float NdotL)
{
	int u, v;
	auto pointN = applyGrid(point, u, v);

	if (mFirst && (u % 2) == (v % 2))
		return mFirst->eval(pointN, L, NdotL);
	else if (mSecond)
		return mSecond->eval(pointN, L, NdotL);

	return Spectrum();
}

ShaderClosure GridMaterial::applyGrid(const ShaderClosure& point, int& u, int& v) const
{
	u = (int)(point.UVW(0) * mGridCount);
	v = (int)(point.UVW(1) * mGridCount);

	if (mTiledUV) {
		ShaderClosure pointN = point;
		pointN.UVW			 = Eigen::Vector3f(point.UVW(1) * mGridCount - u,
									 point.UVW(1) * mGridCount - v,
									 point.UVW(2));
		return pointN;
	} else {
		return point;
	}
}

float GridMaterial::pdf(const ShaderClosure& point, const Eigen::Vector3f& L, float NdotL)
{
	int u, v;
	auto pointN = applyGrid(point, u, v);

	if (mFirst && (u % 2) == (v % 2))
		return mFirst->pdf(pointN, L, NdotL);
	else if (mSecond)
		return mSecond->pdf(pointN, L, NdotL);

	return 0;
}

MaterialSample GridMaterial::sample(const ShaderClosure& point, const Eigen::Vector3f& rnd)
{
	int u, v;
	auto pointN = applyGrid(point, u, v);

	if (mFirst && (u % 2) == (v % 2))
		return mFirst->sample(pointN, rnd);
	else if (mSecond)
		return mSecond->sample(pointN, rnd);

	return MaterialSample();
}

std::string GridMaterial::dumpInformation() const
{
	std::stringstream stream;

	stream << std::boolalpha << Material::dumpInformation()
		   << "  <GridMaterial>:" << std::endl
		   << "    GridCount:  " << gridCount() << std::endl
		   << "    IsUVTiled:  " << tileUV() << std::endl
		   << "    Material 1: " << (mFirst ? mFirst->id() : -1) << std::endl
		   << "    Material 2: " << (mSecond ? mSecond->id() : -1) << std::endl;

	return stream.str();
}
}
