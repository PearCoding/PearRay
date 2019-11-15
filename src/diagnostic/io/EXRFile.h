#pragma once

#include "ImageBufferView.h"
#include <QVector>

#include <memory>

class EXRLayer : public ImageBufferView {
public:
	EXRLayer(const QString& name, size_t channels, size_t width, size_t height);
	virtual ~EXRLayer();

	inline QVector<QVector<float>>& data() { return mData; }
	inline QVector<QString>& channelNames() { return mChannelNames; }

	const QString& name() const override { return mName; }
	size_t width() const override { return mWidth; }
	size_t height() const override { return mHeight; }
	size_t channelCount() const override { return mChannelNames.size(); }
	size_t viewChannelCount() const override { return channelCount(); }
	const QString& channelName(size_t c) const override { return mChannelNames.at((int)c); }

	float value(size_t x, size_t y, size_t channel) const override
	{
		return mData[(int)channel][static_cast<int>(y * mWidth + x)];
	}

	void ensureRightOrder();

private:
	QString mName;
	QVector<QVector<float>> mData;
	QVector<QString> mChannelNames;

	size_t mWidth;
	size_t mHeight;
};

class EXRFile {
public:
	EXRFile();
	~EXRFile();

	bool open(const QString& file);

	inline size_t width() const { return mWidth; }
	inline size_t height() const { return mHeight; }

	inline const QVector<std::shared_ptr<EXRLayer>>& layers() const { return mLayers; }

private:
	size_t mWidth;
	size_t mHeight;
	QVector<std::shared_ptr<EXRLayer>> mLayers;
};