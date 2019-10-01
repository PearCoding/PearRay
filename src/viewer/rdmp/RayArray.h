#pragma once

#include <iostream>

#include <QVector3D>
#include <QVector>

struct Ray {
	QVector3D Origin;
	QVector3D Direction;
};

class RayArray {
public:
	RayArray();
	virtual ~RayArray();

	void clear();
	bool load(std::istream& stream, quint32 step = 1);
	void populate(QVector<QVector3D>& vertices, QVector<unsigned int>& indices) const;

	inline size_t size() const { return mRays.size(); }

private:
	QVector<Ray> mRays;
};