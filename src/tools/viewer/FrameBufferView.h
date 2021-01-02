#pragma once

#include "io/ImageBufferView.h"

namespace PR {
class FrameOutputDevice;
} // namespace PR

class FrameBufferView : public PR::UI::ImageBufferView {
public:
	explicit FrameBufferView(const std::shared_ptr<PR::FrameOutputDevice>& device);
	virtual ~FrameBufferView();

	QString viewName() const override;
	int width() const override;
	int height() const override;
	int channelCount() const override;
	int viewChannelCount() const override;
	QString channelName(int channel) const override;
	float value(int x, int y, int channel) const override;

private:
	const std::shared_ptr<PR::FrameOutputDevice> mFrame;
};