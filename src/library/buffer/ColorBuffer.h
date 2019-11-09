#pragma once

#include "PR_Config.h"

namespace PR {
enum ColorBufferMode {
	CBM_RGB,
	CBM_RGBA
};

class ToneMapper;
class PR_LIB ColorBuffer {
public:
	ColorBuffer(size_t width, size_t height, ColorBufferMode mode = CBM_RGBA);
	virtual ~ColorBuffer();

	ColorBuffer(const ColorBuffer& other) = default;
	ColorBuffer& operator=(const ColorBuffer& other) = default;

	ColorBuffer(ColorBuffer&& other) = default;
	ColorBuffer& operator=(ColorBuffer&& other) = default;

	void map(const ToneMapper& mapper, const float* specIn, size_t samples);
	void mapOnlyMapper(const ToneMapper& mapper, const float* rgbIn);

	void flipY();

	inline size_t width() const { return mData->Width; }
	inline size_t height() const { return mData->Height; }
	inline size_t channels() const { return mode() == CBM_RGB ? 3 : 4; }
	inline ColorBufferMode mode() const { return mData->Mode; }

	inline size_t widthPitch() const { return channels() * channelPitch(); }
	inline size_t heightPitch() const { return width() * widthPitch(); }
	inline size_t channelPitch() const { return 1; }

	inline size_t widthBytePitch() const { return widthPitch() * sizeof(float); }
	inline size_t heightBytePitch() const { return heightPitch() * sizeof(float); }
	inline size_t channelBytePitch() const { return channelPitch() * sizeof(float); }

	inline float at(size_t i) const { return i < width() * height() * channels() ? mData->Ptr[i] : 0.0f; }
	inline const float* ptr() const { return mData->Ptr; }
	inline float* ptr() { return mData->Ptr; }

private:
	struct _Data {
		_Data(size_t width, size_t height, ColorBufferMode mode);
		~_Data();

		size_t Width;
		size_t Height;
		ColorBufferMode Mode;
		float* Ptr;
	};

	std::shared_ptr<_Data> mData;
};
} // namespace PR
