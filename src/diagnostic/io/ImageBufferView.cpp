#include "ImageBufferView.h"
#include "mapper/ToneMapper.h"

static QImage::Format channelToFormat(size_t channelCount)
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

	auto map = [&](float v) { return static_cast<quint8>(mapper.map(v) * 255); };

	// Copy to image
	switch (viewChannelCount()) {
	case 1: // Grayscale
		for (size_t y = 0; y < height(); ++y) {
			uchar* ptr = image.scanLine(y);
			for (size_t x = 0; x < width(); ++x) {
				ptr[x] = map(value(x, y, channelOffset + 0));
			}
		}
		break;
	case 2: // RGB
		for (size_t y = 0; y < height(); ++y) {
			uchar* ptr = image.scanLine(y);
			for (size_t x = 0; x < width(); ++x) {
				ptr[3 * x] = (channelMask & 0x1)
								 ? map(value(x, y, channelOffset + 0))
								 : 0;
				ptr[3 * x + 1] = (channelMask & 0x2)
									 ? map(value(x, y, channelOffset + 1))
									 : 0;
				ptr[3 * x + 2] = 0;
			}
		}
		break;
	case 3: // RGB
		for (size_t y = 0; y < height(); ++y) {
			uchar* ptr = image.scanLine(y);
			for (size_t x = 0; x < width(); ++x) {
				ptr[3 * x] = (channelMask & 0x1)
								 ? map(value(x, y, channelOffset + 0))
								 : 0;
				ptr[3 * x + 1] = (channelMask & 0x2)
									 ? map(value(x, y, channelOffset + 1))
									 : 0;
				ptr[3 * x + 2] = (channelMask & 0x4)
									 ? map(value(x, y, channelOffset + 2))
									 : 0;
			}
		}
		break;
	case 4: // RGBA
		for (size_t y = 0; y < height(); ++y) {
			uchar* ptr = image.scanLine(y);
			for (size_t x = 0; x < width(); ++x) {
				ptr[4 * x] = (channelMask & 0x1)
								 ? map(value(x, y, channelOffset + 0))
								 : 0;
				ptr[4 * x + 1] = (channelMask & 0x2)
									 ? map(value(x, y, channelOffset + 1))
									 : 0;
				ptr[4 * x + 2] = (channelMask & 0x4)
									 ? map(value(x, y, channelOffset + 2))
									 : 0;
				ptr[4 * x + 3] = (channelMask & 0x8)
									 ? map(value(x, y, channelOffset + 3))
									 : 0;
			}
		}
		break;
	}
}

void ImageBufferView::getMinMax(float& min, float& max, quint8 channelMask) const
{
	min = std::numeric_limits<float>::infinity();
	max = -std::numeric_limits<float>::infinity();
	for (size_t y = 0; y < height(); ++y) {
		for (size_t x = 0; x < width(); ++x) {
			for (size_t c = 0; c < channelCount(); ++c) {
				if (!(channelMask & (1 << c)))
					continue;

				float f = value(x, y, c);
				min = std::min(min, f);
				max = std::max(max, f);
			}
		}
	}
}