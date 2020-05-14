#include "EXRFile.h"

#include <OpenImageIO/imageio.h>

#include <QDebug>
#include <QFile>

EXRLayer::EXRLayer(const QString& name, int channels,
				   int width, int height)
	: ImageBufferView()
	, mName(name)
	, mData((int)channels)
	, mChannelNames((int)channels)
	, mWidth(width)
	, mHeight(height)
{
}

EXRLayer::~EXRLayer()
{
}

void EXRLayer::ensureRightOrder()
{
	if (mChannelNames.size() != 3)
		return;

	if (mChannelNames[0] != "B" || mChannelNames[2] != "R")
		return;

	std::swap(mChannelNames[0], mChannelNames[2]);
	std::swap(mData[0], mData[2]);
}

EXRFile::EXRFile()
	: mWidth(0)
	, mHeight(0)
{
}

EXRFile::~EXRFile()
{
}

static QString escapeString(const QString& str)
{
	int s = str.indexOf('[');
	int e = str.lastIndexOf(']');

	if (s < 0 || e < 0)
		return str;
	else {
		QString tmp = str;
		tmp.remove(s, e - s + 1);
		return tmp;
	}
}

static void splitChannelString(const QString& str, QString& parent, QString& child)
{
	int it = str.indexOf('.');
	if (it < 0) {
		parent = str;
		child  = str;
	} else {
		parent = str.left(it);
		child  = str.right(str.size() - it);
	}
}

bool EXRFile::open(const QString& filename)
{
	static const QString SPEC_NAME = "Color";

	const std::string filename_std = filename.toStdString();
	auto file = OIIO::ImageInput::open(filename_std);
	if (!file) {
		qCritical() << "OIIO: Could not open " << filename
					<< ", error = " << OIIO::geterror().c_str();
		return false;
	}

	const OIIO::ImageSpec& spec = file->spec();

	mWidth	 = spec.width;
	mHeight	 = spec.height;
	int size = mWidth * mHeight;

	// Get layers
	auto channels		 = spec.channelnames;
	auto channel_formats = spec.channelformats;

	QMap<QString, QList<QPair<QString, int>>> layerMap;
	int counter = 0;
	for (auto it = channels.begin(); it != channels.end(); ++it) {
		const QString escaped_name = escapeString(QString::fromStdString(*it));
		QString parent, child;
		if (escaped_name == "R" || escaped_name == "G" || escaped_name == "B") {
			parent = SPEC_NAME;
			child  = escaped_name;
		} else {
			splitChannelString(escaped_name, parent, child);
		}
		layerMap[parent] << qMakePair(child, counter);
		++counter;
	}

	mLayers.clear();
	mLayers.reserve(layerMap.size());

	// Iterate through each layer and load them separately
	for (auto it = layerMap.constBegin(); it != layerMap.constEnd(); ++it) {
		std::shared_ptr<EXRLayer> layer = std::make_shared<EXRLayer>(
			it.key(), it.value().size(),
			mWidth, mHeight);

		int nc = 0;
		for (auto channel = it.value().begin(); channel != it.value().end(); ++channel) {
			if (!file->read_image(channel->second, channel->second, OIIO::TypeDesc::FLOAT, layer->data()[nc].data())) {
				qCritical() << "OIIO: Could not read channel " << channel->second << "(" << channel->first << ") from " << filename
							<< ", error = " << OIIO::geterror().c_str();
				return false;
			}

			layer->channelNames()[nc] = channel->first;
			++nc;
		}

		mLayers.push_back(layer);
	}

	if (!file->close()) {
		qCritical() << "OIIO: Could not close " << filename
					<< ", error = " << OIIO::geterror().c_str();
		return false;
	}

	for (auto layer : mLayers)
		layer->ensureRightOrder();
	return true;
}
