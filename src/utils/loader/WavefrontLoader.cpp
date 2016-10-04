#include "WavefrontLoader.h"
#include "geometry/TriMesh.h"
#include "geometry/Face.h"

#include "Environment.h"
#include "Logger.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include "external/tiny_obj_loader.h"

using namespace PR;
namespace PRU
{
	WavefrontLoader::WavefrontLoader(const std::map<std::string, std::string>& overrides) :
		mOverrides(overrides), mScale(1), mFlipNormal(false)
	{
	}

	WavefrontLoader::~WavefrontLoader()
	{
	}

	void WavefrontLoader::load(const std::string& file, Environment* env)
	{
		std::vector<tinyobj::shape_t> shapes;
		std::vector<tinyobj::material_t> materials;// We ignore materials for now
		std::string err;

		if (!tinyobj::LoadObj(shapes, materials, err, file.c_str()))
		{
			PR_LOGGER.instance().logf(L_Error, M_Scene, "Error reading Wavefront file: %s", err.c_str());
		}
		else
		{
			for (tinyobj::shape_t shape : shapes)
			{
				PR::TriMesh* mesh = new PR::TriMesh();
				mesh->reserve(shape.mesh.indices.size() / 3);

				bool hasNorm = shape.mesh.normals.size() > 0;
				bool hasUV = shape.mesh.texcoords.size() > 0;
				float inv = mFlipNormal ? -1.0f : 1.0f;

				for (size_t i = 0; i < shape.mesh.indices.size() / 3; ++i)
				{
					Face* face = new Face;
					for (int j = 0; j < 3; j++)
					{
						int idx = shape.mesh.indices[3 * i + j];
						face->V[j] = PM::pm_Set(mScale * shape.mesh.positions[3 * idx],
							mScale * shape.mesh.positions[3 * idx + 1],
							mScale * shape.mesh.positions[3 * idx + 2],
							1);

						if (hasNorm)
						{
							face->N[j] = PM::pm_Normalize3D(PM::pm_Set(inv * shape.mesh.normals[3 * idx],
								inv * shape.mesh.normals[3 * idx + 1],
								inv * shape.mesh.normals[3 * idx + 2],
								0));
						}

						if (hasUV)
						{
							face->UV[j] = PM::pm_Set(shape.mesh.texcoords[2 * idx],
								shape.mesh.texcoords[2 * idx + 1],
								0, 0);
						}
					}

					mesh->addFace(face);
				}

				if (!hasNorm)
					mesh->calcNormals();

				mesh->build();

				std::string name = shape.name;
				if (mOverrides.count(shape.name))
					name = mOverrides[shape.name];

				int i = 1;
				std::string next = name;
				while (env->hasMesh(next) && i < 1000)
				{
					std::stringstream stream;
					stream << name << "_" << i;
					next = stream.str();
					i++;
				}

				if(env->hasMesh(next))
					PR_LOGGER.logf(L_Error, M_Scene, "Mesh '%s' already in use.", next.c_str());

				env->addMesh(next, mesh);
				PR_LOGGER.logf(L_Info, M_Scene, "Added mesh '%s'", next.c_str());
			}
		}
	}
}