#include "FrameBufferView.h"

#include "output/FrameOutputDevice.h"

using namespace PR;

FrameBufferView::FrameBufferView(const std::shared_ptr<PR::FrameOutputDevice>& device)
	: mFrame(device)
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