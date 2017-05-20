#include "TriMesh.h"
#include "Face.h"
#include "Triangle.h"
#include "container/kdTree.h"
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
			Eigen::Vector3f U = f->V[1] - f->V[0];
			Eigen::Vector3f V = f->V[2] - f->V[0];
			Eigen::Vector3f N = U.cross(V).normalized();

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

	float TriMesh::surfaceArea(uint32 slot, const Eigen::Affine3f& transform) const
	{
		float a = 0;
		for (Face* f : mFaces)
		{
			if(f->MaterialSlot == slot)
			{
				a += Triangle::surfaceArea( transform * f->V[0],
											transform * f->V[1],
											transform * f->V[2]);
			}
		}
		return a;
	}

	float TriMesh::surfaceArea(const Eigen::Affine3f& transform) const
	{
		float a = 0;
		for (Face* f : mFaces)
		{
			a += Triangle::surfaceArea( transform * f->V[0],
										transform * f->V[1],
										transform * f->V[2]);
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
		uint32 fi = Projection::map(ret(0), 0, (int)mFaces.size() - 1);
		auto bary = Projection::triangle(ret(1), ret(2));

		Face* face = mFaces.at(fi);

		Eigen::Vector3f vec;
		Eigen::Vector3f n;
		Eigen::Vector2f uv;
		face->interpolate(bary(0), bary(1), vec, n, uv);

		FaceSample fp;
		fp.P = vec;
		fp.Ng = n;
		fp.UVW = Eigen::Vector3f(uv(0),uv(1),0);
		materialSlot = face->MaterialSlot;

		pdf = 1;//?
		return fp;
	}
}
