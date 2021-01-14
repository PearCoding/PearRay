#include "MtsSerializedLoader.h"
#include "Environment.h"
#include "Logger.h"
#include "Platform.h"
#include "SceneLoadContext.h"
#include "mesh/MeshBase.h"

#include <climits>
#include <fstream>

#include <zlib.h>

namespace PR {
MtsSerializedLoader::MtsSerializedLoader(const std::string& name)
	: mName(name)
	, mScale(1)
	, mFlipNormal(false)
{
}

MtsSerializedLoader::~MtsSerializedLoader()
{
}

constexpr size_t BUFFER_SIZE = 32768;
class CompressedStream {
public:
	inline CompressedStream(std::istream& in, size_t size)
		: mIn(in)
		, mSize(size)
		, mPos(0)
		, mStream()
		, mBuffer()
	{
		mStream.zalloc	 = Z_NULL;
		mStream.zfree	 = Z_NULL;
		mStream.opaque	 = Z_NULL;
		mStream.avail_in = 0;
		mStream.next_in	 = Z_NULL;

		int retval = inflateInit2(&mStream, 15);
		if (retval != Z_OK)
			PR_LOG(L_ERROR) << "Could not initialize ZLIB: " << retval << std::endl;
	}

	template <typename T>
	inline void read(T* ptr)
	{
		size_t size		   = sizeof(T);
		uint8_t* targetPtr = (uint8_t*)ptr;
		while (size > 0) {
			if (mStream.avail_in == 0) {
				size_t remaining = mSize - mPos;
				mStream.next_in	 = mBuffer;
				mStream.avail_in = (uInt)std::min(remaining, sizeof(mBuffer));
				if (mStream.avail_in == 0)
					PR_LOG(L_ERROR) << "Read less data than expected (" << size << " more bytes required)" << std::endl;
				mIn.read(reinterpret_cast<char*>(mBuffer), mStream.avail_in);

				if (!mIn.good())
					PR_LOG(L_ERROR) << "Could not read " << mStream.avail_in << " bytes" << std::endl;

				mPos += mStream.avail_in;
			}

			mStream.avail_out = (uInt)size;
			mStream.next_out  = targetPtr;

			int retval = inflate(&mStream, Z_NO_FLUSH);
			switch (retval) {
			case Z_STREAM_ERROR:
				PR_LOG(L_ERROR) << "inflate(): stream error!" << std::endl;
				break;
			case Z_NEED_DICT:
				PR_LOG(L_ERROR) << "inflate(): need dictionary!" << std::endl;
				break;
			case Z_DATA_ERROR:
				PR_LOG(L_ERROR) << "inflate(): data error!" << std::endl;
				break;
			case Z_MEM_ERROR:
				PR_LOG(L_ERROR) << "inflate(): memory error!" << std::endl;
				break;
			};

			size_t outputSize = size - (size_t)mStream.avail_out;
			targetPtr += outputSize;
			size -= outputSize;

			if (size > 0 && retval == Z_STREAM_END)
				PR_LOG(L_ERROR) << "inflate(): attempting to read past the end of the stream!" << std::endl;
		}
	}

private:
	std::istream& mIn;
	size_t mSize;
	size_t mPos;
	z_stream mStream;
	uint8_t mBuffer[BUFFER_SIZE];
};

enum MeshFlags {
	MF_VERTEXNORMALS = 0x0001,
	MF_TEXCOORDS	 = 0x0002,
	MF_VERTEXCOLORS	 = 0x0008,
	MF_FACENORMALS	 = 0x0010,
	MF_FLOAT		 = 0x1000,
	MF_DOUBLE		 = 0x2000,
};

template <typename T>
void extractArray(std::vector<T>& array, CompressedStream& cin)
{
	for (size_t i = 0; i < array.size(); ++i) {
		T x;
		cin.read(&x);
		array[i] = x;
	}
}

template <typename T>
void ignoreArray(size_t size, CompressedStream& cin)
{
	for (size_t i = 0; i < size; ++i) {
		T _ignore;
		cin.read(&_ignore);
		PR_UNUSED(_ignore);
	}
}

void MtsSerializedLoader::load(const std::filesystem::path& file, SceneLoadContext& ctx)
{
	const uint32 shapeIndex = ctx.parameters().getUInt("shape", 0);

	std::fstream stream(file.c_str(), std::ios::in | std::ios::binary);
	if (!stream) {
		PR_LOG(L_ERROR) << "Could not open mistuba serialized file " << file << std::endl;
		return;
	}

	// Check header
	uint16 fileIdent;
	uint16 fileVersion;
	stream.read(reinterpret_cast<char*>(&fileIdent), sizeof(fileIdent));

	if (fileIdent != 0x041C) {
		PR_LOG(L_ERROR) << "Given file '" << file << "' is not a valid Mitsuba serialized file." << std::endl;
		return;
	}
	stream.read(reinterpret_cast<char*>(&fileVersion), sizeof(fileVersion));
	if (fileVersion < 3) {
		PR_LOG(L_ERROR) << "Given file '" << file << "' has an insufficient version number " << fileVersion << " < 3." << std::endl;
		return;
	}

	// Extract amount of shapes inside the file
	uint32 shapeCount;
	stream.seekg(-std::streamoff(sizeof(shapeCount)), std::ios::end);
	stream.read(reinterpret_cast<char*>(&shapeCount), sizeof(shapeCount));

	if (!stream.good()) {
		PR_LOG(L_ERROR) << "Given file '" << file << "' can not access end of file dictionary." << std::endl;
		return;
	}

	if (shapeIndex >= shapeCount) {
		PR_LOG(L_ERROR) << "Given file '" << file << "' can not access shape index " << shapeIndex << " as it only contains " << shapeCount << " shapes." << std::endl;
		return;
	}

	// Extract mesh file start position
	uint64 shapeFileStart;
	uint64 shapeFileEnd;

	if (fileVersion >= 4) {
		stream.seekg(-std::streamoff(sizeof(shapeCount) + sizeof(shapeFileStart) * (shapeCount - shapeIndex)), std::ios::end);
		stream.read(reinterpret_cast<char*>(&shapeFileStart), sizeof(shapeFileStart));

		if (!stream.good()) {
			PR_LOG(L_ERROR) << "Given file '" << file << "' could not extract shape file offset." << std::endl;
			return;
		}

		// Extract mesh file end position
		if (shapeIndex == shapeCount - 1) {
			stream.seekg(-std::streamoff(sizeof(shapeCount)), std::ios::end);
			shapeFileEnd = stream.tellg();
		} else {
			stream.seekg(-std::streamoff(sizeof(shapeCount) + sizeof(shapeFileEnd) * (shapeCount - shapeIndex + 1)), std::ios::end);
			stream.read(reinterpret_cast<char*>(&shapeFileEnd), sizeof(shapeFileEnd));
		}
	} else { /* Version 3 uses uint32_t instead of uint64_t */
		uint32 _shapeFileStart;
		uint32 _shapeFileEnd;

		stream.seekg(-std::streamoff(sizeof(shapeCount) + sizeof(_shapeFileStart) * (shapeCount - shapeIndex)), std::ios::end);
		stream.read(reinterpret_cast<char*>(&_shapeFileStart), sizeof(_shapeFileStart));

		if (!stream.good()) {
			PR_LOG(L_ERROR) << "Given file '" << file << "' could not extract shape file offset." << std::endl;
			return;
		}

		// Extract mesh file end position
		if (shapeIndex == shapeCount - 1) {
			stream.seekg(-std::streamoff(sizeof(shapeCount)), std::ios::end);
			_shapeFileEnd = stream.tellg();
		} else {
			stream.seekg(-std::streamoff(sizeof(shapeCount) + sizeof(_shapeFileEnd) * (shapeCount - shapeIndex + 1)), std::ios::end);
			stream.read(reinterpret_cast<char*>(&_shapeFileEnd), sizeof(_shapeFileEnd));
		}

		shapeFileStart = _shapeFileStart;
		shapeFileEnd   = _shapeFileEnd;
	}

	if (!stream.good()) {
		PR_LOG(L_ERROR) << "Given file '" << file << "' could not extract shape file offset." << std::endl;
		return;
	}

	const size_t maxContentSize = shapeFileEnd - shapeFileStart - sizeof(uint16) * 2;

	// Go to start position of mesh
	stream.seekg(sizeof(uint16) * 2 /*Header*/ + shapeFileStart, std::ios::beg);

	// Inflate with zlib
	CompressedStream cin(stream, maxContentSize);

	uint32 mesh_flags;
	cin.read(&mesh_flags);

	if (fileVersion >= 4) {
		uint8 utf8Char;
		do {
			cin.read(&utf8Char);
		} while (utf8Char != 0); // Ignore shape name
	}

	uint64 vertexCount;
	uint64 triCount;
	cin.read(&vertexCount);
	cin.read(&triCount);

	if (vertexCount == 0 || triCount == 0) {
		PR_LOG(L_ERROR) << "Given file '" << file << "' has no valid mesh." << std::endl;
		return;
	}

	// Vertices
	std::vector<float> vertices(vertexCount * 3);
	if (mesh_flags & MF_DOUBLE) {
		std::vector<double> tmp(vertexCount * 3);
		extractArray<double>(tmp, cin);
		for (size_t i = 0; i < tmp.size(); ++i)
			vertices[i] = static_cast<float>(tmp[i]);
	} else {
		extractArray<float>(vertices, cin);
	}

	// Normals
	std::vector<float> normals;
	if (mesh_flags & MF_VERTEXNORMALS) {
		normals.resize(vertexCount * 3);
		if (mesh_flags & MF_DOUBLE) {
			std::vector<double> tmp(vertexCount * 3);
			extractArray<double>(tmp, cin);
			for (size_t i = 0; i < tmp.size(); ++i)
				normals[i] = static_cast<float>(tmp[i]);
		} else {
			extractArray<float>(normals, cin);
		}
	}

	// Texcoords
	std::vector<float> texcoords;
	if (mesh_flags & MF_TEXCOORDS) {
		texcoords.resize(vertexCount * 2);
		if (mesh_flags & MF_DOUBLE) {
			std::vector<double> tmp(vertexCount * 2);
			extractArray<double>(tmp, cin);
			for (size_t i = 0; i < tmp.size(); ++i)
				texcoords[i] = static_cast<float>(tmp[i]);
		} else {
			extractArray<float>(texcoords, cin);
		}
	}

	// Vertex Colors (ignored)
	if (mesh_flags & MF_VERTEXCOLORS) {
		if (mesh_flags & MF_DOUBLE)
			ignoreArray<double>(vertexCount * 3, cin);
		else
			ignoreArray<float>(vertexCount * 3, cin);
	}

	// Indices
	std::vector<uint32> indices(triCount * 3);
	if (vertexCount > 0xFFFFFFFF) {
		PR_LOG(L_WARNING) << "No support for 64bit indices." << std::endl;
		std::vector<uint64> tmp(triCount * 3);
		extractArray<uint64>(tmp, cin);
		for (size_t i = 0; i < tmp.size(); ++i)
			indices[i] = static_cast<uint32>(tmp[i]);
	} else {
		extractArray<uint32>(indices, cin);
	}

	// Build
	std::unique_ptr<MeshBase> cnt = std::make_unique<MeshBase>();
	cnt->setVertexComponent(MeshComponent::Vertex, std::move(vertices));
	cnt->setVertexComponentIndices(MeshComponent::Vertex, std::move(indices));
	cnt->setVertexComponent(MeshComponent::Normal, std::move(normals));
	cnt->setVertexComponent(MeshComponent::Texture, std::move(texcoords));

	cnt->assumeTriangular(triCount);

	if (!(mesh_flags & MF_VERTEXNORMALS))
		cnt->buildSmoothNormals();

	std::string errMsg;
	if (!cnt->isValid(&errMsg)) {
		PR_LOG(L_WARNING) << "Mts file could not construct a valid mesh data: " << errMsg << std::endl;
		return;
	}

	PR_LOG(L_INFO) << "Added mesh '" << mName << "' with " << cnt->triangleCount() << " triangles and " << cnt->quadCount() << " quads" << std::endl;
	ctx.addMesh(mName, std::move(cnt));
}
} // namespace PR
