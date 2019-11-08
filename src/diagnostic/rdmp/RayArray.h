#pragma once

#include <QVector3D>
#include <QVector>
#include <QFile>

struct Ray {
	QVector3D Origin;
	QVector3D Direction;
};

class RayArray {
public:
	RayArray();
	virtual ~RayArray();

	void clear();
	bool load(QFile& file, quint32 step = 1);
	void populate(QVector<QVector3D>& vertices,
				  QVector<QVector3D>& colors,
				  QVector<unsigned int>& indices) const;

	inline size_t size() const { return mRays.size(); }

private:
	QVector<Ray> mRays;
};