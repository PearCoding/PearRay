#pragma once

#include "ImageBufferView.h"
#include <QVector>

#include <memory>

class SpecFile : public ImageBufferView {
public:
	SpecFile();
	~SpecFile();

	bool open(const QString& file);

	inline QVector<float>& data() { return mData; }
	inline QVector<QString>& channelNames() { return mChannelNames; }

	const QString& name() const override { return mName; }
	size_t width() const override { return mWidth; }
	size_t height() const override { return mHeight; }
	size_t channelCount() const override { return mChannelNames.size(); }
	size_t viewChannelCount() const override { return 1; }
	const QString& channelName(size_t c) const override { return mChannelNames.at(c); }

	float value(size_t x, size_t y, size_t channel) const override
	{
		return mData[y * mWidth * channelCount() + x * channelCount() + channel];
	}

private:
	QString mName;
	QVector<float> mData;
	QVector<QString> mChannelNames;

	size_t mWidth;
	size_t mHeight;
};