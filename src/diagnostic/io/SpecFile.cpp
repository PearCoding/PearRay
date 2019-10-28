#include "SpecFile.h"

#include <boost/iostreams/device/file.hpp>
#ifdef PR_COMPRESS_SPEC_FILES
#include <boost/iostreams/filter/zlib.hpp>
#endif
#include <boost/iostreams/filtering_stream.hpp>

SpecFile::SpecFile()
	: ImageBufferView()
	, mWidth(0)
	, mHeight(0)
{
}

SpecFile::~SpecFile()
{
}

bool SpecFile::open(const QString& file)
{
	namespace io = boost::iostreams;
	io::filtering_istream in;
#ifdef PR_COMPRESS_SPEC_FILES
	in.push(io::zlib_decompressor());
#endif
	in.push(io::file_source(file.toStdString()));

	char c1, c2, c3, c4;
	in.get(c1);
	in.get(c2);
	in.get(c3);
	in.get(c4);
	if (c1 != 'P' || c2 != 'R' || c3 != '4' || c4 != '2')
		return false;

	quint32 samplingCount;
	quint32 width;
	quint32 height;
	in.read(reinterpret_cast<char*>(&samplingCount), sizeof(samplingCount));
	in.read(reinterpret_cast<char*>(&width), sizeof(width));
	in.read(reinterpret_cast<char*>(&height), sizeof(height));

	mWidth  = width;
	mHeight = height;

	if (samplingCount == 0 || width == 0 || height == 0)
		return false;

	std::vector<float> wavelengths(samplingCount);
	std::vector<float> luminous(samplingCount);
	in.read(reinterpret_cast<char*>(wavelengths.data()), wavelengths.size() * sizeof(float));
	in.read(reinterpret_cast<char*>(luminous.data()), luminous.size() * sizeof(float));

	mChannelNames.resize(samplingCount);
	for (quint32 i = 0; i < samplingCount; ++i) {
		mChannelNames[i] = QString("%1 nm").arg(wavelengths[i], 0, 'f', 2);
	}

	mData.resize(mWidth * mHeight * channelCount());

	for (quint32 y = 0; y < height; ++y) {
		for (quint32 x = 0; x < width; ++x) {
			for (quint32 s = 0; s < samplingCount; ++s) {
				in.read(reinterpret_cast<char*>(
							&mData[y * width * samplingCount + x * samplingCount + s]),
						sizeof(float));
			}
		}
	}

	return true;
}