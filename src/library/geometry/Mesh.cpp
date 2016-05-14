#include "Mesh.h"
#include "Face.h"

namespace PR
{
	Mesh::Mesh()
	{
	}

	Mesh::~Mesh()
	{
		clear();
	}

	void Mesh::addVertex(const PM::vec3& v)
	{
		mBoundingBox.put(v);// FIXME: 0 is already in :/
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
			//TODO
		}
	}
}