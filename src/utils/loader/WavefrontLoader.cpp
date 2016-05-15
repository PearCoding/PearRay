#include "WavefrontLoader.h"
#include "geometry/Mesh.h"
#include "geometry/Face.h"

#include "Logger.h"

#include <sstream>
#include <fstream>
#include <vector>
#include <list>
#include <algorithm>
#include <functional>
#include <cctype>

// Slow implementation, and needs more protections and error messages!
using namespace PR;
namespace PRU
{
	// Utils

	template<typename T>
	T pr_to(const std::string& str)
	{
		T i;
		std::stringstream stream(str);
		stream >> i;
		return i;
	}

	// trim from start
	static inline std::string &ltrim(std::string &s) {
		s.erase(s.begin(), std::find_if(s.begin(), s.end(), std::not1(std::ptr_fun<int, int>(std::isspace))));
		return s;
	}

	// trim from end
	static inline std::string &rtrim(std::string &s) {
		s.erase(std::find_if(s.rbegin(), s.rend(), std::not1(std::ptr_fun<int, int>(std::isspace))).base(), s.end());
		return s;
	}

	// trim from both ends
	static inline std::string &trim(std::string &s) {
		return ltrim(rtrim(s));
	}

	WavefrontLoader::WavefrontLoader() :
		mScale(1)
	{
	}

	WavefrontLoader::~WavefrontLoader()
	{
	}

	struct UniqueVertex
	{
		int VIn;
		int TIn;
		int NIn;
		uint32 ID;//Will be set later
	};

	struct ProxyFace
	{
		std::vector<uint32> uniqueIDs;
	};

	inline bool operator == (const UniqueVertex& f1, const UniqueVertex& f2)
	{
		return f1.VIn == f2.VIn && f1.TIn == f2.TIn && f1.NIn == f2.NIn;
	}

	UniqueVertex unpackFaceVertex(const std::string& str)
	{
		std::string Vert;
		std::string Tex;
		std::string Norm;

		int part = 0;

		for(char c : str)
		{
			if (c == '/')
			{
				part++;

				if (part > 2)
				{
					break;
				}
			}
			else
			{
				switch (part)
				{
				case 0:
					Vert += c;
					break;
				case 1:
					Tex += c;
					break;
				case 2:
					Norm += c;
					break;
				}
			}
		}

		if (Tex.empty())
		{
			Tex = Vert;
		}

		if (Norm.empty())
		{
			Norm = Vert;
		}

		UniqueVertex v;
		v.VIn = pr_to<int>(Vert) - 1;
		v.TIn = pr_to<int>(Tex) - 1;
		v.NIn = pr_to<int>(Norm) - 1;
		v.ID = 0;

		return v;
	}

	void WavefrontLoader::load(const std::string& file, Mesh* mesh)
	{
		PR_ASSERT(mesh);

		std::ifstream input(file, std::ifstream::in);
		std::vector<UniqueVertex> uniqueVertices;
		std::vector<ProxyFace> proxyFaces;

		int lnr = 1;
		std::string line;
		while (std::getline(input, line))
		{
			line = trim(line);
			if (line.empty())
			{
				continue;
			}

			// Parse line
			std::list<std::string> commands;
			std::string tmp;
			for(char c : line)
			{
				if (c == ' ' || c == '\t' || c == '\r')
				{
					if (!tmp.empty())
					{
						commands.push_back(tmp);
						tmp = "";
					}
				}
				else if (c == '#')//Comment
				{
					break;
				}
				else
				{
					tmp += c;
				}
			}

			if (!tmp.empty())
			{
				commands.push_back(tmp);
			}

			if (commands.empty())
			{
				continue;
			}

			// Parse Command
			if (commands.front() == "v")//Vertex
			{
				commands.pop_front();

				PM::vec3 v = PM::pm_Set(0, 0, 0, 1);
				v = PM::pm_SetX(v, pr_to<float>(commands.front()));
				commands.pop_front();
				v = PM::pm_SetY(v, pr_to<float>(commands.front()));
				commands.pop_front();

				if (!commands.empty())
				{
					v = PM::pm_SetZ(v, pr_to<float>(commands.front()));
				}

				mesh->addVertex(v);
			}
			else if (commands.front() == "vt")//Texture
			{
				commands.pop_front();

				PM::vec2 t = PM::pm_Set(0, 0);
				t = PM::pm_SetX(t, pr_to<float>(commands.front()));
				commands.pop_front();
				t = PM::pm_SetY(t, pr_to<float>(commands.front()));
				commands.pop_front();

				mesh->addUV(t);
			}
			else if (commands.front() == "vn")//Normal
			{
				commands.pop_front();

				PM::vec3 n = PM::pm_Set(0, 0, 0);
				n = PM::pm_SetX(n, pr_to<float>(commands.front()));
				commands.pop_front();
				n = PM::pm_SetY(n, pr_to<float>(commands.front()));
				commands.pop_front();

				if (!commands.empty())
				{
					n = PM::pm_SetZ(n, pr_to<float>(commands.front()));
				}

				mesh->addNormal(PM::pm_Normalize3D(n));
			}
			else if (commands.front() == "f")//Face
			{
				commands.pop_front();

				if (commands.size() >= 3)//Triangles or quads
				{
					ProxyFace face;

					const size_t oldSize = commands.size();
					for (size_t i = 0; i < oldSize; ++i)
					{
						UniqueVertex uv = unpackFaceVertex(commands.front());
						commands.pop_front();

						auto it = std::find(uniqueVertices.begin(), uniqueVertices.end(), uv);
						if (it != uniqueVertices.end())
						{
							uv = *it;
						}
						else
						{
							uv.ID = (uint32)uniqueVertices.size();
							uniqueVertices.push_back(uv);
						}

						face.uniqueIDs.push_back(uv.ID);
					}

					proxyFaces.push_back(face);
				}
			}
			//Everything else is ignored

			++lnr;
		}

		// TODO: Only triangles or quads!
		for (ProxyFace f : proxyFaces)
		{
			if (f.uniqueIDs.size() == 3)
			{
				Face* face = new Face;
				UniqueVertex v1 = uniqueVertices.at(f.uniqueIDs.at(0));
				UniqueVertex v2 = uniqueVertices.at(f.uniqueIDs.at(1));
				UniqueVertex v3 = uniqueVertices.at(f.uniqueIDs.at(2));

				face->V1 = mesh->getVertex(v1.VIn);
				face->V2 = mesh->getVertex(v2.VIn);
				face->V3 = mesh->getVertex(v3.VIn);

				if (!mesh->normals().empty())
				{
					face->N1 = mesh->getNormal(v1.NIn);
					face->N2 = mesh->getNormal(v2.NIn);
					face->N3 = mesh->getNormal(v3.NIn);
				}

				if (!mesh->uvs().empty())
				{
					face->UV1 = mesh->getUV(v1.TIn);
					face->UV2 = mesh->getUV(v2.TIn);
					face->UV3 = mesh->getUV(v3.TIn);
				}

				mesh->addFace(face);
			}
			else if (f.uniqueIDs.size() == 4)
			{
				UniqueVertex v1 = uniqueVertices.at(f.uniqueIDs.at(0));
				UniqueVertex v2 = uniqueVertices.at(f.uniqueIDs.at(1));
				UniqueVertex v3 = uniqueVertices.at(f.uniqueIDs.at(2));
				UniqueVertex v4 = uniqueVertices.at(f.uniqueIDs.at(3));

				// First triangle
				Face* face = new Face;
				face->V1 = mesh->getVertex(v1.VIn);
				face->V2 = mesh->getVertex(v2.VIn);
				face->V3 = mesh->getVertex(v4.VIn);

				if (!mesh->normals().empty())
				{
					face->N1 = mesh->getNormal(v1.NIn);
					face->N2 = mesh->getNormal(v2.NIn);
					face->N3 = mesh->getNormal(v4.NIn);
				}

				if (!mesh->uvs().empty())
				{
					face->UV1 = mesh->getUV(v1.TIn);
					face->UV2 = mesh->getUV(v2.TIn);
					face->UV3 = mesh->getUV(v4.TIn);
				}
				mesh->addFace(face);

				// Second triangle
				face = new Face;
				face->V1 = mesh->getVertex(v4.VIn);
				face->V2 = mesh->getVertex(v2.VIn);
				face->V3 = mesh->getVertex(v3.VIn);

				if (!mesh->normals().empty())
				{
					face->N1 = mesh->getNormal(v4.NIn);
					face->N2 = mesh->getNormal(v2.NIn);
					face->N3 = mesh->getNormal(v3.NIn);
				}

				if (!mesh->uvs().empty())
				{
					face->UV1 = mesh->getUV(v4.TIn);
					face->UV2 = mesh->getUV(v2.TIn);
					face->UV3 = mesh->getUV(v3.TIn);
				}
				mesh->addFace(face);
			}
			else
			{
				PR_LOGGER.log(L_Error, M_Internal, "Couldn't add face. Only triangles or quads allowed.");
			}
		}

		mesh->fix();
		mesh->build();

		PR_LOGGER.logf(L_Debug, M_Internal, "V: %d, N: %d, U: %d, F: %d, V: %f",
			mesh->vertices().size(), mesh->normals().size(), mesh->uvs().size(), mesh->faces().size(),
			mesh->boundingBox().volume());
	}
}