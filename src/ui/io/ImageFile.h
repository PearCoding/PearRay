#pragma once

#include "PR_Config.h"
#include <QVector>

#include <memory>

namespace PR {
namespace UI {
class ImageLayer;
class PR_LIB_UI ImageFile {
public:
	ImageFile();
	~ImageFile();

	bool open(const QString& file);

	inline int width() const { return mWidth; }
	inline int height() const { return mHeight; }

	inline const QVector<std::shared_ptr<ImageLayer>>& layers() const { return mLayers; }

private:
	int mWidth;
	int mHeight;
	QVector<std::shared_ptr<ImageLayer>> mLayers;
};
} // namespace UI
} // namespace PR