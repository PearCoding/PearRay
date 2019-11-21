#pragma once

#include <QImage>

class ToneMapper;
class ImageBufferView {
public:
	virtual const QString& name() const						 = 0;
	virtual int width() const								 = 0;
	virtual int height() const								 = 0;
	virtual int channelCount() const						 = 0;
	virtual int viewChannelCount() const					 = 0;
	virtual const QString& channelName(int channel) const = 0;

	virtual float value(int x, int y, int channel) const = 0;

	void fillImage(QImage& image, const ToneMapper& mapper,
				   quint32 channelOffset = 0, quint8 channelMask = 0xFF) const;

	void getMinMax(float& min, float& max, quint8 channelMask = 0xFF) const;
};