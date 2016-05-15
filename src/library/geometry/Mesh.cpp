#include "Mesh.h"
#include "Face.h"
#include "Triangle.h"
#include "scene/kdTree.h"

#include <iterator>

namespace PR
{
	Mesh::Mesh() :
		mKDTree(nullptr)
	{
	}

	Mesh::~Mesh()
	{
		clear();
	}

	void Mesh::addVertex(const PM::vec3& v)
	{
		mBoundingBox.put(v);
		mVertices.push_back(v);
	}

	PM::vec3 Mesh::getVertex(size_t i) const
	{
		return mVertices.at(i);
	}

	void Mesh::addNormal(const PM::vec3& v)
	{
		mNormals.push_back(v);
	}

	PM::vec3 Mesh::getNormal(size_t i) const
	{
		return mNormals.at(i);
	}

	void Mesh::addUV(const PM::vec2& v)
	{
		mUVs.push_back(v);
	}

	PM::vec2 Mesh::getUV(size_t i) const
	{
		return mUVs.at(i);
	}


	void Mesh::addFace(Face* f)
	{
		PR_ASSERT(f);
		mFaces.push_back(f);
	}

	Face* Mesh::getFace(size_t i) const
	{
		return mFaces.at(i);
	}

	void Mesh::clear()
	{
		if (mKDTree)
		{
			delete (PR::kdTree<Face>*)mKDTree;
			mKDTree = nullptr;
		}

		mVertices.clear();
		mNormals.clear();
		mUVs.clear();

		for (Face* f : mFaces)
		{
			delete f;
		}
		mFaces.clear();
	}

	void Mesh::fix()
	{
		if (mNormals.empty())// Calculate normals
		{
			for (Face* f : mFaces)
			{
				PM::vec3 U = PM::pm_Subtract(f->V2, f->V1);
				PM::vec3 V = PM::pm_Subtract(f->V3, f->V1);
				PM::vec3 N = PM::pm_Normalize3D(PM::pm_Cross3D(U, V));

				f->N1 = N;
				f->N2 = N;
				f->N3 = N;
			}
		}

		if (mUVs.empty())// Calculate uvs
		{
			for (Face* f : mFaces)
			{
				f->UV1 = PM::pm_Set(0, 0);
				f->UV2 = PM::pm_Set(0, 0);
				f->UV3 = PM::pm_Set(0, 0);
			}
		}
	}

	void Mesh::build()
	{
		if (mKDTree)
		{
			delete (PR::kdTree<Face>*)mKDTree;
			mKDTree = nullptr;
		}

		mKDTree = new PR::kdTree<Face>(
		[](Face* f) {
			return Triangle::getBoundingBox(f->V1, f->V2, f->V3);
		},
		[](const Ray& ray, FacePoint& point, Face* f, Face*) {
			return Triangle::intersect(ray, *f, point);
		});

		std::list<Face*> list;
		std::copy(mFaces.begin(), mFaces.end(), std::back_inserter(list));
		((PR::kdTree<Face>*)mKDTree)->build(list);
	}
}