#pragma once

#include "ImageBufferView.h"
#include <QVector>

#include <memory>

namespace PR {
namespace UI {
class PR_LIB_UI ImageLayer : public ImageBufferView {
public:
	ImageLayer(const QString& name, const QString& lpe, int channels, int width, int height);
	virtual ~ImageLayer();

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
	QString channelName(int c) const override { return mChannelNames.at((int)c); }

	float value(int x, int y, int channel) const override
	{
		return mData[(int)channel][static_cast<int>(y * mWidth + x)];
	}

	void fillImage(QImage& image, const ImagePipeline& mapper,
				   quint32 channelOffset = 0, quint8 channelMask = 0xFF) const override;

	void ensureRightOrder();

private:
	QString mName;
	QString mLPE;
	QVector<QVector<float>> mData;
	QVector<QString> mChannelNames;

	int mWidth;
	int mHeight;
};
} // namespace UI
} // namespace PR