#pragma once

#include "Serializer.h"

namespace PR {
class Socket;
class PR_LIB_BASE NetworkSerializer : public Serializer {
	PR_CLASS_NON_COPYABLE(NetworkSerializer);

public:
	NetworkSerializer();
	NetworkSerializer(Socket* socket, bool readmode);
	virtual ~NetworkSerializer();

	void setSocket(Socket* socket, bool readmode);
	inline Socket* socket() const { return mSocket; }
	bool hasData() const;

	// Interface
	virtual bool isValid() const override;
	virtual size_t writeRaw(const uint8* data, size_t size) override;
	virtual size_t readRaw(uint8* data, size_t size) override;

private:
	Socket* mSocket;
};
} // namespace PR