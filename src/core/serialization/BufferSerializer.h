#pragma once

#include "Serializer.h"

namespace PR {
class PR_LIB_CORE BufferSerializer : public Serializer {
	PR_CLASS_NON_COPYABLE(BufferSerializer);

public:
	BufferSerializer(uint32 version = 0);
	BufferSerializer(std::vector<uint8>* buffer, bool readmode, uint32 version = 0);
	virtual ~BufferSerializer();

	bool open(std::vector<uint8>* buffer, bool readmode);
	void close();

	// Interface
	virtual bool isValid() const override;
	virtual void writeRaw(const uint8* data, size_t elems, size_t elemSize) override;
	virtual void readRaw(uint8* data, size_t elems, size_t elemSize) override;

private:
	std::vector<uint8>* mBuffer;
	size_t mIt;
};
} // namespace PR