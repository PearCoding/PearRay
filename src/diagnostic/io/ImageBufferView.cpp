#include "ImageBufferView.h"
#include "mapper/ToneMapper.h"

static QImage::Format channelToFormat(int channelCount)
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

void ImageBufferView::fillImage(QImage& image, const ToneMapper& mapper,
								quint32 channelOffset, quint8 channelMask) const
{
	const QImage::Format expectedFormat = channelToFormat(viewChannelCount());
	if (expectedFormat == QImage::Format_Invalid)
		return;

	if (image.width() != (int)width() || image.height() != (int)height()
		|| image.format() != expectedFormat) {
		image = QImage(width(), height(), expectedFormat);
	}

	// Copy to image
	switch (viewChannelCount()) {
	case 1: // Grayscale
		for (int y = 0; y < height(); ++y) {
			uchar* ptr = image.scanLine(y);
			for (int x = 0; x < width(); ++x) {
				const float g = value(x, y, channelOffset + 0);
				ptr[x]		  = static_cast<quint8>(
					   std::max(0.0f, std::min(0.0f, mapper.map(g))) * 255);
			}
		}
		break;
	case 2: // RGB
		for (int y = 0; y < height(); ++y) {
			uchar* ptr = image.scanLine(y);
			for (int x = 0; x < width(); ++x) {
				float cx = mapper.map(value(x, y, channelOffset + 0));
				float cy = mapper.map(value(x, y, channelOffset + 1));
				float cz = 0.0f;
				float cr, cg, cb;
				mapper.formatTriplet(cx, cy, cz, cr, cg, cb);
				ptr[3 * x]	 = (channelMask & 0x1) ? static_cast<quint8>(cr * 255) : 0;
				ptr[3 * x + 1] = (channelMask & 0x2) ? static_cast<quint8>(cg * 255) : 0;
				ptr[3 * x + 2] = (channelMask & 0x4) ? static_cast<quint8>(cb * 255) : 0;
			}
		}
		break;
	case 3: // RGB
		for (int y = 0; y < height(); ++y) {
			uchar* ptr = image.scanLine(y);
			for (int x = 0; x < width(); ++x) {
				float cx = mapper.map(value(x, y, channelOffset + 0));
				float cy = mapper.map(value(x, y, channelOffset + 1));
				float cz = mapper.map(value(x, y, channelOffset + 2));
				float cr, cg, cb;
				mapper.formatTriplet(cx, cy, cz, cr, cg, cb);
				ptr[3 * x]	 = (channelMask & 0x1) ? static_cast<quint8>(cr * 255) : 0;
				ptr[3 * x + 1] = (channelMask & 0x2) ? static_cast<quint8>(cg * 255) : 0;
				ptr[3 * x + 2] = (channelMask & 0x4) ? static_cast<quint8>(cb * 255) : 0;
			}
		}
		break;
	case 4: // RGBA
		for (int y = 0; y < height(); ++y) {
			uchar* ptr = image.scanLine(y);
			for (int x = 0; x < width(); ++x) {
				float cx = mapper.map(value(x, y, channelOffset + 0));
				float cy = mapper.map(value(x, y, channelOffset + 1));
				float cz = mapper.map(value(x, y, channelOffset + 2));
				float cr, cg, cb;
				mapper.formatTriplet(cx, cy, cz, cr, cg, cb);
				const float a  = (channelMask & 0x8) ? value(x, y, channelOffset + 3) : 0;
				ptr[4 * x]	 = (channelMask & 0x1) ? static_cast<quint8>(cr * 255) : 0;
				ptr[4 * x + 1] = (channelMask & 0x2) ? static_cast<quint8>(cg * 255) : 0;
				ptr[4 * x + 2] = (channelMask & 0x4) ? static_cast<quint8>(cb * 255) : 0;
				ptr[4 * x + 3] = static_cast<quint8>(std::max(0.0f, std::min(0.0f, a)) * 255);
			}
		}
		break;
	}
}

void ImageBufferView::getMinMax(float& min, float& max, quint8 channelMask) const
{
	min = std::numeric_limits<float>::infinity();
	max = -std::numeric_limits<float>::infinity();
	for (int y = 0; y < height(); ++y) {
		for (int x = 0; x < width(); ++x) {
			for (int c = 0; c < channelCount(); ++c) {
				if (!(channelMask & (1 << c)))
					continue;

				float f = value(x, y, c);
				min		= std::min(min, f);
				max		= std::max(max, f);
			}
		}
	}
}