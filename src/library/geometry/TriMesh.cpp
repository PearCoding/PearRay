#include "TriMesh.h"
#include "Face.h"
#include "Triangle.h"
#include "scene/kdTree.h"
#include "Random.h"
#include "sampler/Sampler.h"
#include "math/Projection.h"
#include "material/Material.h"

#include <iterator>

namespace PR
{
	typedef PR::kdTree<Face, false> TriKDTree;
	TriMesh::TriMesh() :
		mKDTree(nullptr)
	{
	}

	TriMesh::~TriMesh()
	{
		clear();
	}

	void TriMesh::reserve(size_t count)
	{
		mFaces.reserve(count);
	}

	void TriMesh::setFaces(const std::vector<Face*>& f)
	{
		mFaces = f;
	}

	void TriMesh::addFace(Face* f)
	{
		PR_ASSERT(f);
		mFaces.push_back(f);
	}

	Face* TriMesh::getFace(size_t i) const
	{
		return mFaces.at(i);
	}

	void TriMesh::clear()
	{
		if (mKDTree)
		{
			delete (TriKDTree*)mKDTree;
			mKDTree = nullptr;
		}

		for (Face* f : mFaces)
		{
			delete f;
		}
		mFaces.clear();
	}

	void TriMesh::calcNormals()
	{
		for (Face* f : mFaces)
		{
			PM::vec3 U = PM::pm_Subtract(f->V[1], f->V[0]);
			PM::vec3 V = PM::pm_Subtract(f->V[2], f->V[0]);
			PM::vec3 N = PM::pm_Normalize3D(PM::pm_Cross3D(U, V));

			f->N[0] = N;
			f->N[1] = N;
			f->N[2] = N;
		}
	}

	constexpr float TriangleTestCost = 4.0f;
	void TriMesh::build()
	{
		if (mKDTree)
		{
			delete (TriKDTree*)mKDTree;
			mKDTree = nullptr;
		}

		mKDTree = new TriKDTree(
		[](Face* f) {
			return Triangle::getBoundingBox(f->V[0], f->V[1], f->V[2]);
		},
		[](const Ray& ray, FaceSample& point, Face* f) {
			float t;
			return Triangle::intersect(ray, *f, point, t);// Major bottleneck!
		},
		[](Face* f) {
			return TriangleTestCost;
		});

		std::list<Face*> list;
		std::copy(mFaces.begin(), mFaces.end(), std::back_inserter(list));
		((TriKDTree*)mKDTree)->build(list);

		if(!((TriKDTree*)mKDTree)->isEmpty())
			mBoundingBox = ((TriKDTree*)mKDTree)->boundingBox();
	}

	bool TriMesh::isLight() const
	{
		for (Face* f : mFaces)
		{
			if (f && f->Mat && f->Mat->isLight())
				return true;
		}
		return false;
	}
	
	float TriMesh::surfaceArea(Material* m, const PM::mat& transform) const
	{
		float a = 0;
		for (Face* f : mFaces)
		{
			if(!m || f->Mat == m)
			{
				a += Triangle::surfaceArea(	PM::pm_Multiply(transform, f->V[0]),
											PM::pm_Multiply(transform, f->V[1]),
											PM::pm_Multiply(transform, f->V[2]));
			}
		}
		return a;
	}

	void TriMesh::replaceMaterial(Material* mat)
	{
		for (Face* f : mFaces)
			f->Mat = mat;
	}

	bool TriMesh::checkCollision(const Ray& ray, FaceSample& collisionPoint)
	{
		PR_DEBUG_ASSERT(mKDTree);
		return ((TriKDTree*)mKDTree)->checkCollision(ray, collisionPoint) != nullptr;
	}

	float TriMesh::collisionCost() const
	{
		return TriangleTestCost * ((TriKDTree*)mKDTree)->depth();
	}

	FaceSample TriMesh::getRandomFacePoint(Sampler& sampler, uint32 sample, float& pdf) const
	{
		auto ret = sampler.generate3D(sample);
		uint32 fi = Projection::map(PM::pm_GetX(ret), 0, (int)mFaces.size() - 1);
		auto bary = Projection::triangle(PM::pm_GetY(ret), PM::pm_GetZ(ret));

		Face* face = mFaces.at(fi);

		PM::vec3 vec;
		PM::vec3 n;
		PM::vec2 uv;
		face->interpolate(PM::pm_GetX(bary), PM::pm_GetY(bary), vec, n, uv);

		FaceSample fp;
		fp.P = vec;
		fp.Ng = n;
		fp.UV = uv;
		fp.Material = face->Mat;

		pdf = 1;//?
		return fp;
	}
}