#pragma once

#include "PR_Config.h"

namespace PR {
namespace UI {
struct PR_LIB_UI ImageBufferIOView {
	float* R	   = nullptr;
	Size1i RStride = 1;
	float* G	   = nullptr;
	Size1i GStride = 1;
	float* B	   = nullptr;
	Size1i BStride = 1;
	Size1i Size	   = 0;

	inline bool isValid() const { return R != nullptr && G != nullptr && B != nullptr
										 && RStride > 0 && GStride > 0 && BStride > 0
										 && Size > 0; }
};
} // namespace UI
} // namespace PR