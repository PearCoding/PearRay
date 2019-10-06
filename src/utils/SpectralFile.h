#pragma once

#include "PR_Config.h"

namespace PR {

class Spectrum;
class SpectrumDescriptor;
class PR_LIB_UTILS SpectralFile {
public:
	SpectralFile(const std::shared_ptr<SpectrumDescriptor>& desc, uint32 width, uint32 height);
	SpectralFile(const std::shared_ptr<SpectrumDescriptor>& desc, uint32 width, uint32 height, float* data, bool copy = false);
	virtual ~SpectralFile();

	void set(uint32 row, uint32 column, const Spectrum& spec);
	void get(uint32 row, uint32 column, Spectrum& spec) const;

	void save(const std::string& path, bool compress = true) const;
	static SpectralFile open(const std::string& path, bool compressed = true);

	const std::shared_ptr<SpectrumDescriptor>& descriptor() const;
	uint32 width() const;
	uint32 height() const;
	float* ptr() const;

private:
	std::shared_ptr<struct SF_Data> mData;
};
} // namespace PR
