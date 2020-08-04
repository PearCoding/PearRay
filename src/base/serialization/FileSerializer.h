#pragma once

#include "Serializer.h"

#include <filesystem>

namespace PR {
class PR_LIB_BASE FileSerializer : public Serializer {
	PR_CLASS_NON_COPYABLE(FileSerializer);

public:
	FileSerializer();
	FileSerializer(const std::filesystem::path& path, bool readmode);
	virtual ~FileSerializer();

	bool open(const std::filesystem::path& path, bool readmode);
	void close();

	size_t memoryFootprint() const;

	// Interface
	virtual bool isValid() const override;
	virtual size_t writeRaw(const uint8* data, size_t size) override;
	virtual size_t readRaw(uint8* data, size_t size) override;

private:
	std::unique_ptr<struct FileSerializerInternal> mInternal;
};
} // namespace PR