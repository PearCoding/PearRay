#include "ImageBufferView.h"
#include "mapper/ToneMapper.h"

#ifndef PR_DIAG_NO_TBB
#include "tbb/blocked_range.h"
#include "tbb/parallel_for.h"
#endif

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

#ifndef PR_DIAG_NO_TBB
	tbb::parallel_for(
		tbb::blocked_range<int>(0, height()),
		[&](const tbb::blocked_range<int>& r) {
			const int sy = r.begin();
			const int ey = r.end();
#else
	const int sy = 0;
	const int ey = height();
#endif
			// Copy to image
			switch (viewChannelCount()) {
			case 1: // Grayscale
				for (int y = sy; y < ey; ++y) {
					uchar* ptr = image.scanLine(y);
					for (int x = 0; x < width(); ++x) {
						const float g = value(x, y, channelOffset + 0);
						float lg;
						mapper.mapTriplet(g, g, g, lg, lg, lg);
						ptr[x] = static_cast<quint8>(lg * 255);
					}
				}
				break;
			case 2: // RGB
				for (int y = sy; y < ey; ++y) {
					uchar* ptr = image.scanLine(y);
					for (int x = 0; x < width(); ++x) {
						float cx = value(x, y, channelOffset + 0);
						float cy = value(x, y, channelOffset + 1);
						float cz = 0.0f;
						float cr, cg, cb;
						mapper.mapTriplet(cx, cy, cz, cr, cg, cb);
						ptr[3 * x]	 = (channelMask & 0x1) ? static_cast<quint8>(cr * 255) : 0;
						ptr[3 * x + 1] = (channelMask & 0x2) ? static_cast<quint8>(cg * 255) : 0;
						ptr[3 * x + 2] = (channelMask & 0x4) ? static_cast<quint8>(cb * 255) : 0;
					}
				}
				break;
			case 3: // RGB
				for (int y = sy; y < ey; ++y) {
					uchar* ptr = image.scanLine(y);
					for (int x = 0; x < width(); ++x) {
						float cx = value(x, y, channelOffset + 0);
						float cy = value(x, y, channelOffset + 1);
						float cz = value(x, y, channelOffset + 2);
						float cr, cg, cb;
						mapper.mapTriplet(cx, cy, cz, cr, cg, cb);
						ptr[3 * x]	 = (channelMask & 0x1) ? static_cast<quint8>(cr * 255) : 0;
						ptr[3 * x + 1] = (channelMask & 0x2) ? static_cast<quint8>(cg * 255) : 0;
						ptr[3 * x + 2] = (channelMask & 0x4) ? static_cast<quint8>(cb * 255) : 0;
					}
				}
				break;
			case 4: // RGBA
				for (int y = sy; y < ey; ++y) {
					uchar* ptr = image.scanLine(y);
					for (int x = 0; x < width(); ++x) {
						float cx = value(x, y, channelOffset + 0);
						float cy = value(x, y, channelOffset + 1);
						float cz = value(x, y, channelOffset + 2);
						float cr, cg, cb;
						mapper.mapTriplet(cx, cy, cz, cr, cg, cb);
						const float a  = (channelMask & 0x8) ? value(x, y, channelOffset + 3) : 0;
						ptr[4 * x]	 = (channelMask & 0x1) ? static_cast<quint8>(cr * 255) : 0;
						ptr[4 * x + 1] = (channelMask & 0x2) ? static_cast<quint8>(cg * 255) : 0;
						ptr[4 * x + 2] = (channelMask & 0x4) ? static_cast<quint8>(cb * 255) : 0;
						ptr[4 * x + 3] = static_cast<quint8>(std::max(0.0f, std::min(0.0f, a)) * 255);
					}
				}
				break;
			}
#ifndef PR_DIAG_NO_TBB
		});
#endif
}

void ImageBufferView::getMappedMinMax(float& min, float& max, const ToneMapper& mapper) const
{
	switch (viewChannelCount()) {
	case 1: // Grayscale
	{
		float min1, max1;
		getMinMaxOfChannel(min1, max1, 0);

		float lmin1, lmin2, lmin3;
		mapper.formatOnlyTriplet(min1, min1, min1, lmin1, lmin2, lmin3);
		float lmax1, lmax2, lmax3;
		mapper.formatOnlyTriplet(max1, max1, max1, lmax1, lmax2, lmax3);

		min = lmin1;
		max = lmax1;
	} break;
	case 2: // RGB
	{
		float min1, max1;
		getMinMaxOfChannel(min1, max1, 0);
		float min2, max2;
		getMinMaxOfChannel(min2, max2, 0);

		float lmin1, lmin2, lmin3;
		mapper.formatOnlyTriplet(min1, min2, 0, lmin1, lmin2, lmin3);
		float lmax1, lmax2, lmax3;
		mapper.formatOnlyTriplet(max1, max2, 0, lmax1, lmax2, lmax3);

		min = std::min(lmin1, lmin2);
		max = std::max(lmax1, lmax2);
	} break;
	case 3: // RGB
	case 4: {
		float min1, max1;
		getMinMaxOfChannel(min1, max1, 0);
		float min2, max2;
		getMinMaxOfChannel(min2, max2, 1);
		float min3, max3;
		getMinMaxOfChannel(min3, max3, 2);

		float lmin1, lmin2, lmin3;
		mapper.formatOnlyTriplet(min1, min2, min3, lmin1, lmin2, lmin3);
		float lmax1, lmax2, lmax3;
		mapper.formatOnlyTriplet(max1, max2, max3, lmax1, lmax2, lmax3);

		min = std::min(lmin1, std::min(lmin2, lmin3));
		max = std::max(lmax1, std::max(lmax2, lmax3));
	} break;
	default: // TODO
		min = 0;
		max = 1;
	}
}

void ImageBufferView::getMinMaxOfChannel(float& min, float& max, int channel) const
{
	min = std::numeric_limits<float>::infinity();
	max = -std::numeric_limits<float>::infinity();
	for (int y = 0; y < height(); ++y) {
		for (int x = 0; x < width(); ++x) {
			float f = value(x, y, channel);
			min		= std::min(min, f);
			max		= std::max(max, f);
		}
	}
}