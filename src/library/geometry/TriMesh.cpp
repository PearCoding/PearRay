#include "TriMesh.h"
#include "Face.h"
#include "Triangle.h"
#include "container/kdTree.h"
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
		PR_ASSERT(f, "f has to be not null");
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
			PM::vec3 N = PM::pm_Normalize(PM::pm_Cross(U, V));

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

		((TriKDTree*)mKDTree)->build(mFaces.begin(), mFaces.end(), mFaces.size());

		if(!((TriKDTree*)mKDTree)->isEmpty())
			mBoundingBox = ((TriKDTree*)mKDTree)->boundingBox();
	}

	float TriMesh::surfaceArea(uint32 slot, const PM::mat4& transform) const
	{
		float a = 0;
		for (Face* f : mFaces)
		{
			if(f->MaterialSlot == slot)
			{
				a += Triangle::surfaceArea( PM::pm_Transform(transform, f->V[0]),
											PM::pm_Transform(transform, f->V[1]),
											PM::pm_Transform(transform, f->V[2]));
			}
		}
		return a;
	}

	float TriMesh::surfaceArea(const PM::mat4& transform) const
	{
		float a = 0;
		for (Face* f : mFaces)
		{
			a += Triangle::surfaceArea(	PM::pm_Transform(transform, f->V[0]),
										PM::pm_Transform(transform, f->V[1]),
										PM::pm_Transform(transform, f->V[2]));
		}
		return a;
	}

	Face* TriMesh::checkCollision(const Ray& ray, FaceSample& collisionPoint)
	{
		//PR_ASSERT(mKDTree, "kdTree has to be valid");
		return ((TriKDTree*)mKDTree)->checkCollision(ray, collisionPoint);
	}

	float TriMesh::collisionCost() const
	{
		return TriangleTestCost * ((TriKDTree*)mKDTree)->depth();
	}

	FaceSample TriMesh::getRandomFacePoint(Sampler& sampler, uint32 sample, uint32& materialSlot, float& pdf) const
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
		fp.UVW = PM::pm_ExtendTo3D(uv);
		materialSlot = face->MaterialSlot;

		pdf = 1;//?
		return fp;
	}
}
