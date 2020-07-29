#pragma once

#include "Serializer.h"

namespace PR {
class PR_LIB_BASE BufferSerializer : public Serializer {
	PR_CLASS_NON_COPYABLE(BufferSerializer);

public:
	BufferSerializer();
	BufferSerializer(Serializer* source, size_t bufferSize = 1024);
	virtual ~BufferSerializer();

	inline size_t maxSize() const { return mBuffer.size(); }

	void flush();

	// Interface
	virtual bool isValid() const override;
	virtual size_t writeRaw(const uint8* data, size_t size) override;
	virtual size_t readRaw(uint8* data, size_t size) override;

protected:
	void reset(Serializer* source, size_t bufferSize);

private:
	void fetch();

	Serializer* mSource;
	std::vector<uint8> mBuffer;
	size_t mIt;
	size_t mAvailableIt; // Iterator showing available data, which is the whole buffer in case of write-mode all the time. This is only useful in read-mode
};
} // namespace PR