#pragma once

#include "Serializer.h"

namespace PR {
class PR_LIB_CORE BufferSerializer : public Serializer {
	PR_CLASS_NON_COPYABLE(BufferSerializer);

public:
	BufferSerializer();
	BufferSerializer(uint8* buffer, size_t size, bool readmode);
	virtual ~BufferSerializer();

	inline size_t maxSize() const { return mSize; }

	bool open(uint8* buffer, size_t size, bool readmode);
	void close();

	// Interface
	virtual bool isValid() const override;
	virtual void writeRaw(const uint8* data, size_t elems, size_t elemSize) override;
	virtual void readRaw(uint8* data, size_t elems, size_t elemSize) override;

private:
	uint8* mBuffer;
	size_t mSize;
	size_t mIt;
};
} // namespace PR