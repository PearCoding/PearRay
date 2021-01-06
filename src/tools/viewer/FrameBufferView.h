#pragma once

#include "buffer/FrameBuffer.h"
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
	void fillImage(QImage& image, const PR::UI::ImagePipeline& mapper,
				   quint32 channelOffset = 0, quint8 channelMask = 0xFF) const override;

private:
	const std::shared_ptr<PR::FrameOutputDevice> mFrame;
	const std::shared_ptr<PR::FrameBufferFloat> mWeightedBuffer;
	const std::shared_ptr<PR::FrameBufferFloat> mOutput;
};