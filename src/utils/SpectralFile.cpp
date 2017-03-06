#include "SpectralFile.h"

#include <boost/iostreams/device/file.hpp>
#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iostreams/filter/zlib.hpp>

namespace PR
{
	SpectralFile::SpectralFile() : mData(nullptr) {}

	SpectralFile::SpectralFile(uint32 width, uint32 height)
	{
		mData = new _Data;
		mData->Ptr = new Spectrum[width*height];
		mData->External = false;
		mData->Width = width;
		mData->Height = height;
		mData->Refs = 1;
	}

	SpectralFile::SpectralFile(uint32 width, uint32 height, Spectrum* data, bool copy)
	{
		mData = new _Data;
		mData->Width = width;
		mData->Height = height;
		mData->Refs = 1;

		if(copy)
		{
			mData->Ptr = new Spectrum[width*height];
			mData->External = false;
		}
		else
		{
			mData->Ptr = data;
			mData->External = true;
		}
	}

	SpectralFile::SpectralFile(const SpectralFile& other) : mData(other.mData)
	{
		mData->Refs += 1;
	}

	SpectralFile::SpectralFile(SpectralFile&& other) : mData(other.mData)
	{
		other.mData = nullptr;
	}

	SpectralFile::SpectralFile::~SpectralFile()
	{
		deref();
	}
	
	SpectralFile& SpectralFile::operator = (const SpectralFile& other)
	{
		deref();
		mData = other.mData;
		mData->Refs += 1;

		return *this;
	}

	SpectralFile& SpectralFile::operator = (SpectralFile&& other)
	{
		if(this == &other)
			return *this;
		
		deref();
		mData = other.mData;
		other.mData = nullptr;

		return *this;
	}

	void SpectralFile::set(uint32 px, uint32 py, const Spectrum& spec)
	{
		PR_ASSERT(mData, "Access to invalid spectral file");
		PR_ASSERT(px < mData->Width && py < mData->Height, "Access out of bound");

		mData->Ptr[py*mData->Width + py] = spec;
	}

	const Spectrum& SpectralFile::at(uint32 px, uint32 py) const
	{
		PR_ASSERT(mData, "Access to invalid spectral file");
		PR_ASSERT(px < mData->Width && py < mData->Height, "Access out of bound");

		return mData->Ptr[py*mData->Width + py];
	}

	void SpectralFile::save(const std::string& path) const
	{
		PR_ASSERT(mData, "Access to invalid spectral file");

		namespace io = boost::iostreams;
		io::filtering_ostream out;
    	out.push(io::zlib_compressor());
    	out.push(io::file_sink(path));

		out.put('P').put('R').put('4').put('2');

		uint32 tmp = Spectrum::SAMPLING_COUNT;
		out.write(reinterpret_cast<const char*>(&tmp), sizeof(uint32));
		tmp = mData->Width;
		out.write(reinterpret_cast<const char*>(&tmp), sizeof(uint32));
		tmp = mData->Height;
		out.write(reinterpret_cast<const char*>(&tmp), sizeof(uint32));

		for (uint32 i = 0; i < mData->Height * mData->Width; ++i)
			out.write(reinterpret_cast<const char*>(&mData->Ptr[i]),
				Spectrum::SAMPLING_COUNT * sizeof(float));
	}

	SpectralFile SpectralFile::open(const std::string& path)
	{
		namespace io = boost::iostreams;
		io::filtering_istream in;
    	in.push(io::zlib_decompressor());
    	in.push(io::file_source(path));

		char c1,c2,c3,c4;
		in.get(c1); in.get(c2); in.get(c3); in.get(c4);
		if(c1 != 'P' || c2 != 'R' || c3 != '4' || c4 != '2')
			return SpectralFile();
		
		uint32 samplingCount;
		uint32 width;
		uint32 height;
		in.read(reinterpret_cast<char*>(&samplingCount), sizeof(uint32));
		in.read(reinterpret_cast<char*>(&width), sizeof(width));
		in.read(reinterpret_cast<char*>(&height), sizeof(height));

		if(samplingCount != Spectrum::SAMPLING_COUNT || width == 0 || height == 0)
			return SpectralFile();
		
		SpectralFile f(width, height);
		for (uint32 y = 0; y < height; ++y)
		{
			for (uint32 x = 0; x < width; ++x)
			{
				Spectrum spec;
				in.read(reinterpret_cast<char*>(&spec),
					Spectrum::SAMPLING_COUNT * sizeof(float));
				f.set(x,y,spec);
			}
		}

		return f;
	}

	void SpectralFile::deref()
	{
		if(!mData)
			return;
		
		PR_ASSERT(mData->Refs > 0, "Reference can't be zero prior dereferencing");
		mData->Refs -= 1;

		if(mData->Refs == 0)
		{
			if(mData->External)
				delete[] mData->Ptr;

			delete mData;
		}

		mData = nullptr;
	}
}