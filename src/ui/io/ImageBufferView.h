#pragma once

#include <QImage>

#include "PR_Config.h"

namespace PRUI {
class ToneMapper;

// TODO: Split this into channel count specific classes!
class PR_LIB_UI ImageBufferView {
public:
	virtual QString viewName() const					  = 0;
	virtual int width() const							  = 0;
	virtual int height() const							  = 0;
	virtual int channelCount() const					  = 0;
	virtual int viewChannelCount() const				  = 0;
	virtual const QString& channelName(int channel) const = 0;

	virtual float value(int x, int y, int channel) const = 0;

	void fillImage(QImage& image, const ToneMapper& mapper,
				   quint32 channelOffset = 0, quint8 channelMask = 0xFF) const;

	void getMappedMinMax(float& min, float& max, const ToneMapper& mapper) const;
	void getMinMaxOfChannel(float& min, float& max, int channel) const;
};
}