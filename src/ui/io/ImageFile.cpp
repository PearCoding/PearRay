#include "ImageFile.h"
#include "ImageLayer.h"

#include <OpenImageIO/imageio.h>

#include <QDebug>
#include <QFile>

namespace PR {
namespace UI {
ImageFile::ImageFile()
	: mWidth(0)
	, mHeight(0)
{
}

ImageFile::~ImageFile()
{
}

static void splitChannelLPE(const QString& str, QString& lpe, QString& remainder)
{
	int s = str.indexOf('[');
	int e = str.lastIndexOf(']');

	if (s < 0 || e < 0) {
		lpe		  = "";
		remainder = str;
	} else {
		lpe		  = str.mid(s + 1, e - s - 1); // Skip [ and ]
		remainder = str;
		remainder.remove(s, e - s + 1);
	}
}

static void splitChannelString(const QString& str, QString& parent, QString& lpe, QString& child)
{
	QString tmp;
	splitChannelLPE(str, lpe, tmp);

	int it = tmp.indexOf('.');
	if (it < 0) {
		parent = "";
		child  = tmp;
	} else {
		parent = tmp.left(it);
		child  = tmp.right(tmp.size() - it - 1);
	}
}

bool ImageFile::open(const QString& filename)
{
	static const QString SPEC_NAME = "Color";

	const std::string filename_std = filename.toStdString();
	auto file					   = OIIO::ImageInput::open(filename_std);
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

	using LayerKey	 = QPair<QString, QString>;
	using LayerValue = QPair<QString, int>;
	QMap<LayerKey, QList<LayerValue>> layerMap;
	int counter = 0;
	for (auto it = channels.begin(); it != channels.end(); ++it) {
		QString parent, child, lpe;
		splitChannelString(QString::fromStdString(*it), parent, lpe, child);

		if (parent.isEmpty()) {
			if (child == "R" || child == "G" || child == "B")
				parent = SPEC_NAME;
			else
				parent = child;
		}

		layerMap[qMakePair(parent, lpe)] << qMakePair(child, counter);
		++counter;
	}

	mLayers.clear();
	mLayers.reserve(layerMap.size());

	// Iterate through each layer and load them separately
	for (auto it = layerMap.constBegin(); it != layerMap.constEnd(); ++it) {
		std::shared_ptr<ImageLayer> layer = std::make_shared<ImageLayer>(
			it.key().first, it.key().second, it.value().size(),
			mWidth, mHeight);

		int nc = 0;
		for (auto channel = it.value().begin(); channel != it.value().end(); ++channel) {
			layer->data()[nc].resize(size);

#if OIIO_PLUGIN_VERSION < 21
			if (!file->read_image(channel->second, channel->second, OIIO::TypeDesc::FLOAT, layer->data()[nc].data())) {
#else
			if (!file->read_image(0, 0, channel->second, channel->second, OIIO::TypeDesc::FLOAT, layer->data()[nc].data())) {
#endif
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

#if OIIO_PLUGIN_VERSION < 22
		OIIO::ImageInput::destroy(file);
#endif
		return false;
	}

#if OIIO_PLUGIN_VERSION < 22
	OIIO::ImageInput::destroy(file);
#endif

	for (auto layer : mLayers)
		layer->ensureRightOrder();
	return true;
}
} // namespace UI
} // namespace PR