#pragma once

#include "BufferSerializer.h"
#include "NetworkSerializer.h"

namespace PR {
class PR_LIB_BASE BufferedNetworkSerializer : public BufferSerializer {
	PR_CLASS_NON_COPYABLE(BufferedNetworkSerializer);

public:
	BufferedNetworkSerializer();
	BufferedNetworkSerializer(Socket* socket, bool readmode, size_t bufferSize = 4096);
	virtual ~BufferedNetworkSerializer();

	inline Socket* socket() const { return mNetwork.socket(); }
	inline bool hasData() const { return mNetwork.hasData(); }

private:
	NetworkSerializer mNetwork;
};
} // namespace PR