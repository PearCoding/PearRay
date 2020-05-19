#include "SpecFile.h"

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
	Q_UNUSED(file);
	
	// TODO
	return false;
}