#pragma once

#include <QImage>
#include <QVector>

#include <memory>

class SpecFile {
public:
	SpecFile();
	~SpecFile();

	bool open(const QString& file);

	inline QVector<float>& data() { return mData; }
	inline QVector<QString>& channelNames() { return mChannelNames; }
	inline const QString& name() const { return mName; }
	inline size_t width() const { return mWidth; }
	inline size_t height() const { return mHeight; }
	inline size_t channelCount() const { return mChannelNames.size(); }

	inline float value(size_t x, size_t y, size_t channel) const
	{
		return mData[y * mWidth * channelCount() + x * channelCount() + channel];
	}

	void fillImage(QImage& image, quint32 channel) const;

private:
	QString mName;
	QVector<float> mData;
	QVector<QString> mChannelNames;

	size_t mWidth;
	size_t mHeight;
};