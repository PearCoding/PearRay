#include "SpecFile.h"

#include <boost/iostreams/device/file.hpp>
#include <boost/iostreams/filter/zlib.hpp>
#include <boost/iostreams/filtering_stream.hpp>

SpecFile::SpecFile()
	: mWidth(0)
	, mHeight(0)
{
}

SpecFile::~SpecFile()
{
}

void SpecFile::fillImage(QImage& image, quint32 channel) const
{
	const QImage::Format expectedFormat = QImage::Format_Grayscale8;
	if (expectedFormat == QImage::Format_Invalid)
		return;

	if (image.width() != (int)mWidth || image.height() != (int)mHeight
		|| image.format() != expectedFormat) {
		image = QImage(mWidth, mHeight, expectedFormat);
	}

	// Retrieve image wide maximum/minimum
	float minimum = std::numeric_limits<float>::infinity();
	float maximum = 0;
	for (size_t i = 0; i < (size_t)width() * height(); ++i) {
		const float f = mData[i * channelCount() + channel];
		minimum		  = std::min(minimum, std::abs(f));
		maximum		  = std::max(maximum, std::abs(f));
	}

	if (minimum < 1)
		minimum = 0;

	if (maximum < 1)
		maximum = 1;

	const float scale = 255 / (maximum - minimum);

	auto map = [&](float v) { return (std::abs(v) - minimum) * scale; };

	for (size_t y = 0; y < mHeight; ++y) {
		uchar* ptr = image.scanLine(y);
		for (size_t x = 0; x < mWidth; ++x) {
			ptr[x] = map(mData[y * mWidth * channelCount() + x * channelCount() + channel]);
		}
	}
}

bool SpecFile::open(const QString& file)
{
	namespace io = boost::iostreams;
	io::filtering_istream in;
	in.push(io::zlib_decompressor());
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