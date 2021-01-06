#pragma once

#include <QImage>

#include "ImageBufferIOView.h"

namespace PR {
namespace UI {
class ImagePipeline;

// TODO: Split this into channel count specific classes!
class PR_LIB_UI ImageBufferView {
public:
	virtual QString viewName() const			   = 0;
	virtual int width() const					   = 0;
	virtual int height() const					   = 0;
	virtual int channelCount() const			   = 0;
	virtual int viewChannelCount() const		   = 0;
	virtual QString channelName(int channel) const = 0;

	virtual float value(int x, int y, int channel) const = 0;

	virtual void fillImage(QImage& image, const ImagePipeline& mapper,
						   quint32 channelOffset = 0, quint8 channelMask = 0xFF) const = 0;

	static inline QImage::Format channelToFormat(int channelCount)
	{
		switch (channelCount) {
		case 1:
			return QImage::Format_Grayscale8;
		case 2:
		case 3:
			return QImage::Format_RGB888;
		case 4:
			return QImage::Format_RGBA8888;
		default:
			return QImage::Format_Invalid;
		}
	}
};
} // namespace UI
} // namespace PR