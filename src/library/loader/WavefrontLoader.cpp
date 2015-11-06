#include "WavefrontLoader.h"
#include "geometry/Mesh.h"
#include "geometry/Normal.h"
#include "geometry/UV.h"
#include "geometry/Vertex.h"
#include "geometry/Face.h"

#include <sstream>
#include <fstream>
#include <vector>
#include <list>
#include <algorithm>
#include <functional>
#include <cctype>

// Slow implementation, and needs more protections and error messages!
namespace PR
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
		v.TIn = pr_to<int>(Vert) - 1;
		v.NIn = pr_to<int>(Vert) - 1;
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
		while (!std::getline(input, line))
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

				Vertex* v = new Vertex;
				v->X = pr_to<float>(commands.front());
				commands.pop_front();
				v->Y = pr_to<float>(commands.front());
				commands.pop_front();

				if (commands.empty())
				{
					v->Z = 0;
				}
				else
				{
					v->Z = pr_to<float>(commands.front());
				}

				mesh->addVertex(v);
			}
			else if (commands.front() == "vt")//Texture
			{
				commands.pop_front();

				UV* t = new UV;
				t->U = pr_to<float>(commands.front());
				commands.pop_front();
				t->V = pr_to<float>(commands.front());
				commands.pop_front();

				mesh->addUV(t);
			}
			else if (commands.front() == "vn")//Normal
			{
				commands.pop_front();

				Normal* n = new Normal;
				n->X = pr_to<float>(commands.front());
				commands.pop_front();
				n->Y = pr_to<float>(commands.front());
				commands.pop_front();

				if (commands.empty())
				{
					n->Z = 0;
				}
				else
				{
					n->Z = pr_to<float>(commands.front());
				}

				mesh->addNormal(n);
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
							uv.ID = uniqueVertices.size();
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

		for (ProxyFace f : proxyFaces)
		{
			Face* face = new Face;
			for (uint32 i : f.uniqueIDs)
			{
				UniqueVertex v = uniqueVertices.at(i);
				
				Vertex* vert = mesh->getVertex(v.VIn);
				Normal* norm = mesh->getNormal(v.NIn);
				UV* uv = mesh->getUV(v.TIn);

				face->addVertex(vert);
				face->addNormal(norm);
				face->addUV(uv);
			}
		}
	}
}