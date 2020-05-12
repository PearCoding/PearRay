#pragma once

#include "PR_Config.h"

namespace PR {

class Spectrum;
class SpectrumDescriptor;
class PR_LIB_CORE SpectralFile {
public:
	SpectralFile() = default;
	SpectralFile(const std::shared_ptr<SpectrumDescriptor>& desc, size_t width, size_t height);
	SpectralFile(const std::shared_ptr<SpectrumDescriptor>& desc, size_t width, size_t height, float* data, bool copy = false);
	virtual ~SpectralFile();

	void set(size_t row, size_t column, const Spectrum& spec);
	void get(size_t row, size_t column, Spectrum& spec) const;

	void save(const std::wstring& path, bool compress = true) const;
	static SpectralFile open(const std::wstring& path, bool compressed = true);

	const std::shared_ptr<SpectrumDescriptor>& descriptor() const;
	size_t width() const;
	size_t height() const;
	float* ptr() const;

private:
	std::shared_ptr<struct SF_Data> mData;
};
} // namespace PR
