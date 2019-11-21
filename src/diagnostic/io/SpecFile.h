#pragma once

#include "ImageBufferView.h"
#include <QVector>

#include <memory>

class SpecFile : public ImageBufferView {
public:
	SpecFile();
	virtual ~SpecFile();

	bool open(const QString& file);

	inline QVector<float>& data() { return mData; }
	inline QVector<QString>& channelNames() { return mChannelNames; }

	const QString& name() const override { return mName; }
	int width() const override { return mWidth; }
	int height() const override { return mHeight; }
	int channelCount() const override { return mChannelNames.size(); }
	int viewChannelCount() const override { return 1; }
	const QString& channelName(int c) const override { return mChannelNames.at(c); }

	float value(int x, int y, int channel) const override
	{
		return mData[y * mWidth * channelCount() + x * channelCount() + channel];
	}

private:
	QString mName;
	QVector<float> mData;
	QVector<QString> mChannelNames;

	int mWidth;
	int mHeight;
};