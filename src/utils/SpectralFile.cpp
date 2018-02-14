#include "SpectralFile.h"
#include "spectral/SpectrumDescriptor.h"

#include <boost/iostreams/device/file.hpp>
#include <boost/iostreams/filter/zlib.hpp>
#include <boost/iostreams/filtering_stream.hpp>
#include <stdexcept>

namespace PR {

struct SF_Data {
	std::shared_ptr<SpectrumDescriptor> Descriptor;
	uint32 Width;
	uint32 Height;
	bool External;
	float* Ptr;

	SF_Data(const std::shared_ptr<SpectrumDescriptor>& desc, uint32 width, uint32 height)
		: Descriptor(desc)
		, Width(width)
		, Height(height)
		, External(false)
	{
		Ptr = new float[width * height * desc->samples()];
	}

	SF_Data(const std::shared_ptr<SpectrumDescriptor>& desc, uint32 width, uint32 height, float* data)
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
};

SpectralFile::SpectralFile(const std::shared_ptr<SpectrumDescriptor>& desc, uint32 width, uint32 height)
	: mData(std::make_shared<SF_Data>(desc, width, height))
{
}

SpectralFile::SpectralFile(const std::shared_ptr<SpectrumDescriptor>& desc, uint32 width, uint32 height, float* data, bool copy)
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

void SpectralFile::set(uint32 row, uint32 column, const Spectrum& spec)
{
	PR_ASSERT(column < mData->Width && row < mData->Height, "Access out of bound");

	spec.copyTo(&mData->Ptr[row * mData->Width * mData->Descriptor->samples() + column * mData->Descriptor->samples()]);
}

void SpectralFile::get(uint32 row, uint32 column, Spectrum& spec) const
{
	PR_ASSERT(column < mData->Width && row < mData->Height, "Access out of bound");

	spec.copyFrom(&mData->Ptr[row * mData->Width * mData->Descriptor->samples() + column * mData->Descriptor->samples()]);
}

void SpectralFile::save(const std::string& path) const
{
	PR_ASSERT(mData, "Access to invalid spectral file");

	namespace io = boost::iostreams;
	io::filtering_ostream out;
	out.push(io::zlib_compressor());
	out.push(io::file_sink(path));

	out.put('P').put('R').put('4').put('2');

	uint32 tmp = mData->Descriptor->samples();
	out.write(reinterpret_cast<const char*>(&tmp), sizeof(uint32));
	tmp = mData->Width;
	out.write(reinterpret_cast<const char*>(&tmp), sizeof(uint32));
	tmp = mData->Height;
	out.write(reinterpret_cast<const char*>(&tmp), sizeof(uint32));

	for (uint32 i = 0; i < mData->Height * mData->Width * mData->Descriptor->samples(); ++i)
		out.write(reinterpret_cast<const char*>(&mData->Ptr[i]), sizeof(float));
}

SpectralFile SpectralFile::open(const std::string& path)
{
	namespace io = boost::iostreams;
	io::filtering_istream in;
	in.push(io::zlib_decompressor());
	in.push(io::file_source(path));

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

	std::shared_ptr<SpectrumDescriptor> desc = std::make_shared<SpectrumDescriptor>(samplingCount, 0, 0); // TODO: Get lambda information

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

uint32 SpectralFile::width() const
{
	return mData ? mData->Width : 0;
}

uint32 SpectralFile::height() const
{
	return mData ? mData->Height : 0;
}

float* SpectralFile::ptr() const
{
	return mData ? mData->Ptr : nullptr;
}
} // namespace PR