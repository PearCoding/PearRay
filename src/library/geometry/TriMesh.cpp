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
			delete (PR::kdTree<Face>*)mKDTree;
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

	void TriMesh::build()
	{
		if (mKDTree)
		{
			delete (PR::kdTree<Face>*)mKDTree;
			mKDTree = nullptr;
		}

		mKDTree = new PR::kdTree<Face>(
		[](Face* f) {
			return Triangle::getBoundingBox(f->V[0], f->V[1], f->V[2]);
		},
		[](const Ray& ray, FacePoint& point, float& t, Face* f, Face*) {
			return Triangle::intersect(ray, *f, point, t);
		});

		std::list<Face*> list;
		std::copy(mFaces.begin(), mFaces.end(), std::back_inserter(list));
		((PR::kdTree<Face>*)mKDTree)->build(list);

		mBoundingBox = ((PR::kdTree<Face>*)mKDTree)->root()->boundingBox;
	}

	bool TriMesh::isLight() const
	{
		for (Face* f : mFaces)
		{
			if (f && f->Mat->isLight())
				return true;
		}
		return false;
	}

	void TriMesh::replaceMaterial(Material* mat)
	{
		for (Face* f : mFaces)
		{
			f->Mat = mat;
		}
	}

	bool TriMesh::checkCollision(const Ray& ray, FacePoint& collisionPoint, float& t)
	{
		PR_DEBUG_ASSERT(mKDTree);
		return ((PR::kdTree<Face>*)mKDTree)->checkCollision(ray, collisionPoint, t) != nullptr;
	}

	FacePoint TriMesh::getRandomFacePoint(Sampler& sampler, Random& random, uint32 sample) const
	{
		auto ret = sampler.generate3D(sample);
		uint32 fi = Projection::map(PM::pm_GetX(ret), 0, (int)mFaces.size() - 1);

		Face* face = mFaces.at(fi);

		PM::vec3 vec;
		PM::vec3 n;
		PM::vec2 uv;
		face->interpolate(PM::pm_GetY(ret), PM::pm_GetZ(ret), vec, n, uv);

		FacePoint fp;
		fp.setVertex(vec);
		fp.setNormal(n);
		fp.setUV(uv);
		fp.setMaterial(face->Mat);

		return fp;
	}
}