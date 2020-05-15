#pragma once

#include "Serializer.h"

namespace PR {
class PR_LIB_CORE FileSerializer : public Serializer {
	PR_CLASS_NON_COPYABLE(FileSerializer);

public:
	FileSerializer();
	FileSerializer(const std::wstring& path, bool readmode);
	virtual ~FileSerializer();

	bool open(const std::wstring& path, bool readmode);
	void close();

	size_t memoryFootprint() const;

	// Interface
	virtual bool isValid() const override;
	virtual void writeRaw(const uint8* data, size_t elems, size_t elemSize) override;
	virtual void readRaw(uint8* data, size_t elems, size_t elemSize) override;

private:
	std::unique_ptr<struct FileSerializerInternal> mInternal;
};
} // namespace PR