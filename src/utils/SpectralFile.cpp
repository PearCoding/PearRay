#include "SpectralFile.h"
#include "spectral/Spectrum.h"
#include "spectral/SpectrumDescriptor.h"

#include <boost/iostreams/device/file_descriptor.hpp>
#ifdef PR_COMPRESS_SPEC_FILES
#include <boost/iostreams/filter/zlib.hpp>
#endif
#include <boost/iostreams/filtering_stream.hpp>
#include <boost/filesystem.hpp>
#include <stdexcept>

namespace PR {

struct SF_Data {
	std::shared_ptr<SpectrumDescriptor> Descriptor;
	size_t Width;
	size_t Height;
	bool External;
	float* Ptr;

	SF_Data(const std::shared_ptr<SpectrumDescriptor>& desc, size_t width, size_t height)
		: Descriptor(desc)
		, Width(width)
		, Height(height)
		, External(false)
	{
		Ptr = new float[width * height * desc->samples()];
	}

	SF_Data(const std::shared_ptr<SpectrumDescriptor>& desc, size_t width, size_t height, float* data)
		: Descriptor(desc)
		, Width(width)
		, Height(height)
		, External(true)
		, Ptr(data)
	{
	}

	~SF_Data()
	{
		if (!External) {
			delete[] Ptr;
		}
	}

	SF_Data(const SF_Data&) = default;
	SF_Data(SF_Data&&)		= default;
	SF_Data& operator=(const SF_Data&) = default;
	SF_Data& operator=(SF_Data&&) = default;
};

SpectralFile::SpectralFile(const std::shared_ptr<SpectrumDescriptor>& desc, size_t width, size_t height)
	: mData(std::make_shared<SF_Data>(desc, width, height))
{
}

SpectralFile::SpectralFile(const std::shared_ptr<SpectrumDescriptor>& desc, size_t width, size_t height, float* data, bool copy)
{
	if (copy) {
		mData = std::make_shared<SF_Data>(desc, width, height);
		std::memcpy(mData->Ptr, data, width * height * desc->samples() * sizeof(float));
	} else {
		mData = std::make_shared<SF_Data>(desc, width, height, data);
	}
}

SpectralFile::~SpectralFile()
{
}

void SpectralFile::set(size_t row, size_t column, const Spectrum& spec)
{
	PR_ASSERT(column < mData->Width && row < mData->Height, "Access out of bound");

	spec.copyTo(&mData->Ptr[row * mData->Width * mData->Descriptor->samples() + column * mData->Descriptor->samples()]);
}

void SpectralFile::get(size_t row, size_t column, Spectrum& spec) const
{
	PR_ASSERT(column < mData->Width && row < mData->Height, "Access out of bound");

	spec.copyFrom(&mData->Ptr[row * mData->Width * mData->Descriptor->samples() + column * mData->Descriptor->samples()]);
}

void SpectralFile::save(const std::wstring& path, bool compress) const
{
	PR_ASSERT(mData, "Access to invalid spectral file");

	namespace io = boost::iostreams;
	io::filtering_ostream out;
#ifdef PR_COMPRESS_SPEC_FILES
	if (compress)
		out.push(io::zlib_compressor());
#endif
	out.push(io::file_descriptor_sink(boost::filesystem::wpath(path)));

	out.put('P').put('R').put('4').put('2');

	// Image header
	uint32 tmp = mData->Descriptor->samples();
	out.write(reinterpret_cast<const char*>(&tmp), sizeof(uint32));
	tmp = (uint32)mData->Width;
	out.write(reinterpret_cast<const char*>(&tmp), sizeof(uint32));
	tmp = (uint32)mData->Height;
	out.write(reinterpret_cast<const char*>(&tmp), sizeof(uint32));

	// Wavelength data
	const auto& wavelengths = mData->Descriptor->getWavelengths();
	out.write(reinterpret_cast<const char*>(wavelengths.data()),
			  wavelengths.size() * sizeof(float));

	// Luminous factor data
	const auto& lmbs = mData->Descriptor->getLuminousFactors();
	out.write(reinterpret_cast<const char*>(lmbs.data()),
			  lmbs.size() * sizeof(float));

	// Image content
	out.write(reinterpret_cast<const char*>(mData->Ptr), sizeof(float) * mData->Height * mData->Width * mData->Descriptor->samples());
}

SpectralFile SpectralFile::open(const std::wstring& path, bool compressed)
{
	namespace io = boost::iostreams;
	io::filtering_istream in;
#ifdef PR_COMPRESS_SPEC_FILES
	if (compressed)
		in.push(io::zlib_decompressor());
#endif
	in.push(io::file_descriptor_source(boost::filesystem::wpath(path)));

	char c1, c2, c3, c4;
	in.get(c1);
	in.get(c2);
	in.get(c3);
	in.get(c4);
	if (c1 != 'P' || c2 != 'R' || c3 != '4' || c4 != '2')
		throw std::runtime_error("Invalid file");

	uint32 samplingCount;
	uint32 width;
	uint32 height;
	in.read(reinterpret_cast<char*>(&samplingCount), sizeof(uint32));
	in.read(reinterpret_cast<char*>(&width), sizeof(width));
	in.read(reinterpret_cast<char*>(&height), sizeof(height));

	if (samplingCount == 0 || width == 0 || height == 0)
		throw std::runtime_error("Invalid file");

	std::vector<float> wavelengths(samplingCount);
	std::vector<float> luminous(samplingCount);
	in.read(reinterpret_cast<char*>(wavelengths.data()), wavelengths.size() * sizeof(float));
	in.read(reinterpret_cast<char*>(luminous.data()), luminous.size() * sizeof(float));

	std::shared_ptr<SpectrumDescriptor> desc = std::make_shared<SpectrumDescriptor>(wavelengths, luminous);

	SpectralFile f(desc, width, height);
	for (uint32 y = 0; y < height; ++y) {
		for (uint32 x = 0; x < width; ++x) {
			for (uint32 s = 0; s < samplingCount; ++s) {
				in.read(reinterpret_cast<char*>(&f.mData->Ptr[y * width * samplingCount + x * width * samplingCount + s]), sizeof(float));
			}
		}
	}

	return f;
}

const std::shared_ptr<SpectrumDescriptor>& SpectralFile::descriptor() const
{
	return mData->Descriptor;
}

size_t SpectralFile::width() const
{
	return mData ? mData->Width : 0;
}

size_t SpectralFile::height() const
{
	return mData ? mData->Height : 0;
}

float* SpectralFile::ptr() const
{
	return mData ? mData->Ptr : nullptr;
}
} // namespace PR
