#ifndef PR_DIAG_NO_TBB
#include "tbb/blocked_range.h"
#include "tbb/parallel_for.h"
#endif

#include "FrameBufferView.h"
#include "mapper/ImagePipeline.h"

#include "output/FrameOutputDevice.h"

using namespace PR;

FrameBufferView::FrameBufferView(const std::shared_ptr<PR::FrameOutputDevice>& device)
	: mFrame(device)
	, mWeightedBuffer(FrameBufferFloat::sameAsPtr(*mFrame->data().getInternalChannel_Spectral(AOV_Output)))
	, mOutput(FrameBufferFloat::sameAsPtr(*mFrame->data().getInternalChannel_Spectral(AOV_Output)))
{
}

FrameBufferView::~FrameBufferView()
{
}

QString FrameBufferView::viewName() const
{
	return "Output";
}

int FrameBufferView::width() const
{
	return mFrame->data().getInternalChannel_Spectral(AOV_Output)->width();
}

int FrameBufferView::height() const
{
	return mFrame->data().getInternalChannel_Spectral(AOV_Output)->height();
}

int FrameBufferView::channelCount() const { return 3; }

int FrameBufferView::viewChannelCount() const { return 3; }

QString FrameBufferView::channelName(int channel) const
{
	switch (channel) {
	case 0:
		return "R";
	case 1:
		return "G";
	case 2:
		return "B";
	default:
		return "Unknown";
	}
}

// TODO: Faster approach?
float FrameBufferView::value(int x, int y, int channel) const
{
	const float weight = mFrame->data().getInternalChannel_1D(AOV_PixelWeight)->getFragment(Point2i(x, y), 0);
	if (weight > PR_EPSILON)
		return mFrame->data().getInternalChannel_Spectral(AOV_Output)->getFragment(Point2i(x, y), channel) / weight;
	else
		return 0.0f;
}

void FrameBufferView::fillImage(QImage& image, const PR::UI::ImagePipeline& mapper,
								quint32 channelOffset, quint8 channelMask) const
{
	const QImage::Format expectedFormat = channelToFormat(viewChannelCount());
	if (expectedFormat == QImage::Format_Invalid)
		return;

	if (image.width() != (int)width() || image.height() != (int)height()
		|| image.format() != expectedFormat) {
		image = QImage(width(), height(), expectedFormat);
	}

	// Update output buffer
	const auto& frame_output = mFrame->data().getInternalChannel_Spectral(AOV_Output);
	const auto& frame_weight = mFrame->data().getInternalChannel_1D(AOV_PixelWeight);

#ifndef PR_DIAG_NO_TBB
	tbb::parallel_for(tbb::blocked_range<int>(0, mWeightedBuffer->size().area()), [&](const tbb::blocked_range<int>& r) {
		const int si = r.begin();
		const int ei = r.end();
#else
	const int si = 0;
	const int ei = mWeightedBuffer->size().area();
	PR_OPT_LOOP
#endif
		for (Size1i i = si; i < ei; ++i) {
			const float w = frame_weight->getFragment(i, 0);
			if (w > PR_EPSILON) {
				const float iw = 1 / w;
				PR_UNROLL_LOOP(3)
				for (Size1i k = 0; k < 3; ++k)
					mWeightedBuffer->getFragment(i, k) = frame_output->getFragment(i, k) * iw;
			}
		}
#ifndef PR_DIAG_NO_TBB
	});
#endif

	// Construct buffer descriptors
	const UI::ImageBufferIOView weightedBufferDesc{
		&mWeightedBuffer->getFragment(0, 0), mWeightedBuffer->widthPitch(),
		&mWeightedBuffer->getFragment(0, 1), mWeightedBuffer->widthPitch(),
		&mWeightedBuffer->getFragment(0, 2), mWeightedBuffer->widthPitch(),
		mWeightedBuffer->size().area()
	};

	const UI::ImageBufferIOView outputDesc{
		&mOutput->getFragment(0, 0), mOutput->widthPitch(),
		&mOutput->getFragment(0, 1), mOutput->widthPitch(),
		&mOutput->getFragment(0, 2), mOutput->widthPitch(),
		mOutput->size().area()
	};

	mapper.mapTriplet(weightedBufferDesc, outputDesc);

	const auto pixel = [this](int x, int y, int c) {
		return mOutput->getFragment(Point2i(x, y), c);
	};

#ifndef PR_DIAG_NO_TBB
	tbb::parallel_for(tbb::blocked_range<int>(0, height()), [&](const tbb::blocked_range<int>& r) {
		const int sy = r.begin();
		const int ey = r.end();
#else
	const int sy = 0;
	const int ey = height();
#endif
		for (int y = sy; y < ey; ++y) {
			uchar* ptr = image.scanLine(y);
			for (int x = 0; x < width(); ++x) {
				float cx	   = pixel(x, y, channelOffset + 0);
				float cy	   = pixel(x, y, channelOffset + 1);
				float cz	   = pixel(x, y, channelOffset + 2);
				ptr[3 * x + 0] = (channelMask & 0x1) ? static_cast<quint8>(cx * 255) : 0;
				ptr[3 * x + 1] = (channelMask & 0x2) ? static_cast<quint8>(cy * 255) : 0;
				ptr[3 * x + 2] = (channelMask & 0x4) ? static_cast<quint8>(cz * 255) : 0;
			}
		}
#ifndef PR_DIAG_NO_TBB
	});
#endif
}