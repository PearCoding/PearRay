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

bool RayArray::load(std::istream& stream)
{
	quint64 size;
	stream.read((char*)&size, sizeof(size));

	mRays.reserve(mRays.size() + size);
	for (size_t i = 0; i < size; ++i) {
		Ray ray;
		stream.read((char*)&ray.Origin[0], sizeof(float));
		stream.read((char*)&ray.Origin[1], sizeof(float));
		stream.read((char*)&ray.Origin[2], sizeof(float));
		stream.read((char*)&ray.Direction[0], sizeof(float));
		stream.read((char*)&ray.Direction[1], sizeof(float));
		stream.read((char*)&ray.Direction[2], sizeof(float));

		mRays.append(ray);
	}

	return true;
}

void RayArray::populate(QVector<QVector3D>& vertices, QVector<unsigned int>& indices) const
{
	vertices.clear();
	indices.clear();

	vertices.reserve(mRays.size() * 2);
	indices.reserve(mRays.size() * 2);

	for (const Ray& ray : mRays) {
		size_t id = vertices.size();
		vertices.append(ray.Origin);
		vertices.append(ray.Origin + ray.Direction);

		indices.append(id);
		indices.append(id + 1);
	}
}
