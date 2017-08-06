#pragma once

#include "PR_Config.h"

namespace PR {
enum ColorBufferMode {
	CBM_RGB,
	CBM_RGBA
};

class Spectrum;
class ToneMapper;
class PR_LIB ColorBuffer {
public:
	ColorBuffer(uint32 width, uint32 height, ColorBufferMode mode = CBM_RGBA);
	ColorBuffer(const ColorBuffer& other);
	ColorBuffer(ColorBuffer&& other);
	virtual ~ColorBuffer();

	ColorBuffer& operator=(const ColorBuffer& other);
	ColorBuffer& operator=(ColorBuffer&& other);

	void map(const ToneMapper& mapper, const Spectrum* specIn);
	void mapOnlyMapper(const ToneMapper& mapper, const float* rgbIn);

	inline uint32 width() const { return mData->Width; }
	inline uint32 height() const { return mData->Height; }
	inline ColorBufferMode mode() const { return mData->Mode; }

	inline float* ptr() { return mData->Ptr; }

private:
	struct _Data {
		_Data(uint32 width, uint32 height, ColorBufferMode mode);
		~_Data();

		ColorBufferMode Mode;
		uint32 Width;
		uint32 Height;
		float* Ptr;
	};

	std::shared_ptr<_Data> mData;
};
}
