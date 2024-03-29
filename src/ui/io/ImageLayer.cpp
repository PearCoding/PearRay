#ifndef PR_DIAG_NO_TBB
#include "tbb/blocked_range.h"
#include "tbb/parallel_for.h"
#endif

#include "ImageLayer.h"
#include "mapper/ImagePipeline.h"

namespace PR {
namespace UI {
ImageLayer::ImageLayer(const QString& name, const QString& lpe, int channels,
					   int width, int height)
	: ImageBufferView()
	, mName(name)
	, mLPE(lpe)
	, mData((int)channels)
	, mChannelNames((int)channels)
	, mWidth(width)
	, mHeight(height)
{
}

ImageLayer::~ImageLayer()
{
}

void ImageLayer::ensureRightOrder()
{
	if (mChannelNames.size() != 3)
		return;

	if (mChannelNames[0] != "B" || mChannelNames[2] != "R")
		return;

	std::swap(mChannelNames[0], mChannelNames[2]);
	std::swap(mData[0], mData[2]);
}

void ImageLayer::fillImage(QImage& image, const ImagePipeline& mapper,
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
						ptr[3 * x + 0] = (channelMask & 0x1) ? static_cast<quint8>(cr * 255) : 0;
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
						ptr[3 * x + 0] = (channelMask & 0x1) ? static_cast<quint8>(cr * 255) : 0;
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
						ptr[4 * x + 0] = (channelMask & 0x1) ? static_cast<quint8>(cr * 255) : 0;
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

} // namespace UI
} // namespace PR