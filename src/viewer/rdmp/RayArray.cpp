#include "RayArray.h"

RayArray::RayArray()
{
}

RayArray::~RayArray()
{
}

void RayArray::clear()
{
	mRays.clear();
}

bool RayArray::load(std::istream& stream, quint32 step)
{
	quint64 size;
	stream.read((char*)&size, sizeof(size));
	size /= step;

	if (size == 0)
		size = 1;

	mRays.reserve(mRays.size() + size);

	const std::streamoff off = (step - 1) * sizeof(float) * 6;
	for (quint32 i = 0; i < size; ++i) {
		Ray ray;
		stream.read((char*)&ray.Origin[0], sizeof(float));
		stream.read((char*)&ray.Origin[1], sizeof(float));
		stream.read((char*)&ray.Origin[2], sizeof(float));
		stream.read((char*)&ray.Direction[0], sizeof(float));
		stream.read((char*)&ray.Direction[1], sizeof(float));
		stream.read((char*)&ray.Direction[2], sizeof(float));

		mRays.append(ray);

		if (step > 1)
			stream.seekg(off, std::ios::cur);
	}

	return true;
}

void RayArray::populate(QVector<QVector3D>& vertices, QVector<unsigned int>& indices) const
{
	constexpr float L = 0.2f;

	vertices.clear();
	indices.clear();

	vertices.reserve(mRays.size() * 2);
	indices.reserve(mRays.size() * 2);

	for (const Ray& ray : mRays) {
		size_t id = vertices.size();
		vertices.append(ray.Origin);
		vertices.append(ray.Origin + ray.Direction * L);

		indices.append(id);
		indices.append(id + 1);
	}
}
