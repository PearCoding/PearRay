#pragma once

#include <QImage>
#include <QVector>

#include <memory>

class EXRLayer {
public:
	EXRLayer(const QString& name, size_t channels, size_t width, size_t height);
	~EXRLayer();

	inline QVector<QVector<float>>& data() { return mData; }
	inline QVector<QString>& channelNames() { return mChannelNames; }
	inline const QString& name() const { return mName; }
	inline size_t width() const { return mWidth; }
	inline size_t height() const { return mHeight; }

	inline float value(size_t x, size_t y, size_t channel) const
	{
		return mData[channel][y * mWidth + x];
	}

	void fillImage(QImage& image, quint8 channelMask = 0xFF) const;

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