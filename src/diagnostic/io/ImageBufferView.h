#pragma once

#include <QImage>

class ToneMapper;
class ImageBufferView {
public:
	virtual const QString& name() const						 = 0;
	virtual size_t width() const							 = 0;
	virtual size_t height() const							 = 0;
	virtual size_t channelCount() const						 = 0;
	virtual size_t viewChannelCount() const					 = 0;
	virtual const QString& channelName(size_t channel) const = 0;

	virtual float value(size_t x, size_t y, size_t channel) const = 0;

	void fillImage(QImage& image, const ToneMapper& mapper,
				   quint32 channelOffset = 0, quint8 channelMask = 0xFF) const;

	void getMinMax(float& min, float& max, quint8 channelMask = 0xFF) const;
};