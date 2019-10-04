#include "EXRFile.h"

#include <OpenEXR/ImfChannelList.h>
#include <OpenEXR/ImfInputFile.h>
#include <OpenEXR/ImfTestFile.h>

#include <QDebug>

EXRLayer::EXRLayer(const QString& name, size_t channels,
				   size_t width, size_t height)
	: mName(name)
	, mData(channels)
	, mChannelNames(channels)
	, mWidth(width)
	, mHeight(height)
{
}

EXRLayer::~EXRLayer()
{
}

static QImage::Format channelToFormat(size_t channelCount)
{
	switch (channelCount) {
	case 1:
		return QImage::Format_Grayscale8;
	case 2:
	case 3:
		return QImage::Format_RGB888;
	case 4:
		return QImage::Format_RGBA8888;
	default:
		return QImage::Format_Invalid;
	}
}

void EXRLayer::fillImage(QImage& image, quint8 channelMask) const
{
	const QImage::Format expectedFormat = channelToFormat(mData.size());
	if (expectedFormat == QImage::Format_Invalid)
		return;

	if (image.width() != (int)mWidth || image.height() != (int)mHeight
		|| image.format() != expectedFormat) {
		image = QImage(mWidth, mHeight, expectedFormat);
	}

	// Retrieve image wide maximum
	float maximum = 0;
	for (size_t i = 0; i < (size_t)mData.size(); ++i) {
		if (!((1 << i) & channelMask))
			continue;

		for (float f : mData[i])
			maximum = std::max(maximum, f);
	}

	float scale = 255;
	if (maximum > 1)
		scale /= maximum;

	// Copy to image
	switch (mData.size()) {
	case 1: // Grayscale
		for (size_t y = 0; y < mHeight; ++y) {
			uchar* ptr = image.scanLine(y);
			for (size_t x = 0; x < mWidth; ++x) {
				ptr[x] = mData[0][y * mWidth + x] * scale;
			}
		}
		break;
	case 2: // RGB
		for (size_t y = 0; y < mHeight; ++y) {
			uchar* ptr = image.scanLine(y);
			for (size_t x = 0; x < mWidth; ++x) {
				ptr[3 * x]	 = (channelMask & 0x1) ? mData[0][y * mWidth + x] * scale : 0;
				ptr[3 * x + 1] = (channelMask & 0x2) ? mData[1][y * mWidth + x] * scale : 0;
				ptr[3 * x + 2] = 0;
			}
		}
		break;
	case 3: // RGB
		for (size_t y = 0; y < mHeight; ++y) {
			uchar* ptr = image.scanLine(y);
			for (size_t x = 0; x < mWidth; ++x) {
				ptr[3 * x]	 = (channelMask & 0x1) ? mData[0][y * mWidth + x] * scale : 0;
				ptr[3 * x + 1] = (channelMask & 0x2) ? mData[1][y * mWidth + x] * scale : 0;
				ptr[3 * x + 2] = (channelMask & 0x4) ? mData[2][y * mWidth + x] * scale : 0;
			}
		}
		break;
	case 4: // RGBA
		for (size_t y = 0; y < mHeight; ++y) {
			uchar* ptr = image.scanLine(y);
			for (size_t x = 0; x < mWidth; ++x) {
				ptr[4 * x]	 = (channelMask & 0x1) ? mData[0][y * mWidth + x] * scale : 0;
				ptr[4 * x + 1] = (channelMask & 0x2) ? mData[1][y * mWidth + x] * scale : 0;
				ptr[4 * x + 2] = (channelMask & 0x4) ? mData[2][y * mWidth + x] * scale : 0;
				ptr[4 * x + 3] = (channelMask & 0x8) ? mData[3][y * mWidth + x] * scale : 0;
			}
		}
		break;
	}
}

//////////////////////////////////

EXRFile::EXRFile()
	: mWidth(0)
	, mHeight(0)
{
}

EXRFile::~EXRFile()
{
}

bool EXRFile::open(const QString& filename)
{
	using namespace OPENEXR_IMF_NAMESPACE;
	using namespace IMATH_NAMESPACE;

	const std::string filenameS = filename.toStdString();
	bool tiled					= false;

	if (!isOpenExrFile(filenameS.c_str(), tiled))
		return false;
	if (tiled)
		return false;

	InputFile file(filenameS.c_str());

	/*if (!file.isComplete())
		return false;*/

	// Get Header information
	Box2i dw = file.header().dataWindow();
	mWidth   = dw.max.x - dw.min.x + 1;
	mHeight  = dw.max.y - dw.min.y + 1;

	size_t size = mWidth * mHeight;

	// Get layers
	const ChannelList& channels = file.header().channels();
	QMap<QString, QStringList> layerMap;

	if (channels.find("R") != channels.end()
		|| channels.find("G") != channels.end()
		|| channels.find("B") != channels.end()) {
		QStringList list;
		if (channels.find("R") != channels.end())
			list << "R";
		if (channels.find("G") != channels.end())
			list << "G";
		if (channels.find("B") != channels.end())
			list << "B";

		layerMap["Color"] = list;
	}

	for (auto i = channels.begin(); i != channels.end(); ++i) {
		QString name = i.name();
		if (name != "R" && name != "G" && name != "B" && !name.contains('.')) {
			layerMap[name] = QStringList(name);
		}
	}

	std::set<std::string> layers;
	channels.layers(layers);
	for (std::set<string>::const_iterator i = layers.begin(); i != layers.end(); ++i) {
		ChannelList::ConstIterator layerBegin, layerEnd;
		channels.channelsInLayer(*i, layerBegin, layerEnd);

		QStringList list;
		for (ChannelList::ConstIterator j = layerBegin; j != layerEnd; ++j) {
			list << j.name();
		}

		layerMap[QString::fromStdString(*i)] = list;
	}

	mLayers.clear();
	mLayers.reserve(layerMap.size());

	// Iterate through each layer and load them separately
	for (auto it = layerMap.constBegin(); it != layerMap.constEnd(); ++it) {
		std::shared_ptr<EXRLayer> layer = std::make_shared<EXRLayer>(
			it.key(), it.value().size(),
			mWidth, mHeight);

		// Fill framebuffer with layer.channel information
		QVector<QPair<int, half*>> halfFormat;
		FrameBuffer frameBuffer;

		size_t k = 0;
		for (QString ch : it.value()) {
			QString plainChannelName;
			int pointPos = ch.indexOf('.');
			if (pointPos < 0) {
				plainChannelName = ch;
			} else {
				plainChannelName = ch.mid(pointPos + 1);
			}

			layer->channelNames()[k] = plainChannelName;
			layer->data()[k].resize(size);

			const Channel* channel = channels.findChannel(ch.toStdString());
			float* ptr			   = layer->data()[k].data();

			switch (channel->type) {
			case FLOAT:
				frameBuffer.insert(ch.toStdString(),
								   Slice(FLOAT,
										 (char*)(ptr - dw.min.x - dw.min.y * mWidth),
										 sizeof(ptr[0]) * 1,
										 sizeof(ptr[0]) * mWidth,
										 1, 1,
										 0.0));
				break;
			case HALF: {
				QPair<int, half*> p;
				p.first  = k;
				p.second = new half[size];
				halfFormat.append(p);

				frameBuffer.insert(ch.toStdString(),
								   Slice(HALF,
										 (char*)(p.second - dw.min.x - dw.min.y * mWidth),
										 sizeof(p.second[0]) * 1,
										 sizeof(p.second[0]) * mWidth,
										 1, 1,
										 0.0));
			} break;
			case UINT:
				qCritical() << "EXR: No support for uint channels";
				break;
			default:
				break;
			}

			++k;
		}

		// Load framebuffer
		file.setFrameBuffer(frameBuffer);
		file.readPixels(dw.min.y, dw.max.y);

		// Convert half to float!
		for (auto p : halfFormat) {
			float* ptr = layer->data()[p.first].data();

			for (size_t i = 0; i < size; ++i) {
				ptr[i] = p.second[i];
			}
			delete[] p.second;
		}

		mLayers.push_back(layer);
	}

	return true;
}
