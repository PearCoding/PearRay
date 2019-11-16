#pragma once

#include <QObject>

class IValueProvider : public QObject {
	Q_OBJECT
public:
	inline explicit IValueProvider(QObject* parent)
		: QObject(parent)
	{
	}

	virtual ~IValueProvider() = default;

	virtual float xAt(int index) const = 0;
	virtual float yAt(int index) const = 0;
	virtual int elementCount() const   = 0;

signals:
	void dataChanged();
};