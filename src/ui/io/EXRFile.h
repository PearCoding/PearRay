#pragma once

#include "ImageBufferView.h"
#include <QVector>

#include <memory>

namespace PRUI {
class PR_LIB_UI EXRLayer : public ImageBufferView {
public:
	EXRLayer(const QString& name, const QString& lpe, int channels, int width, int height);
	virtual ~EXRLayer();

	inline QVector<QVector<float>>& data() { return mData; }
	inline QVector<QString>& channelNames() { return mChannelNames; }

	QString viewName() const override
	{
		if (mLPE.isEmpty())
			return mName;
		else
			return mName + " [" + mLPE + "]";
	}

	int width() const override { return mWidth; }
	int height() const override { return mHeight; }
	int channelCount() const override { return mChannelNames.size(); }
	int viewChannelCount() const override { return channelCount(); }
	const QString& channelName(int c) const override { return mChannelNames.at((int)c); }

	float value(int x, int y, int channel) const override
	{
		return mData[(int)channel][static_cast<int>(y * mWidth + x)];
	}

	void ensureRightOrder();

private:
	QString mName;
	QString mLPE;
	QVector<QVector<float>> mData;
	QVector<QString> mChannelNames;

	int mWidth;
	int mHeight;
};

class EXRFile {
public:
	EXRFile();
	~EXRFile();

	bool open(const QString& file);

	inline int width() const { return mWidth; }
	inline int height() const { return mHeight; }

	inline const QVector<std::shared_ptr<EXRLayer>>& layers() const { return mLayers; }

private:
	int mWidth;
	int mHeight;
	QVector<std::shared_ptr<EXRLayer>> mLayers;
};
}