#include "NetworkSerializer.h"
#include "Logger.h"
#include "network/Socket.h"

namespace PR {
NetworkSerializer::NetworkSerializer()
	: Serializer(false)
	, mSocket(nullptr)
{
}

NetworkSerializer::NetworkSerializer(Socket* socket, bool readmode)
	: Serializer(readmode)
	, mSocket(socket)
{
	PR_ASSERT(socket, "Expected a valid Socket");
}

NetworkSerializer::~NetworkSerializer()
{
}

void NetworkSerializer::setSocket(Socket* socket, bool readmode)
{
	PR_ASSERT(socket, "Expected a valid Socket");
	mSocket = socket;
	setReadMode(readmode);
}

bool NetworkSerializer::isValid() const { return mSocket != nullptr; }
bool NetworkSerializer::hasData() const { return isValid() && mSocket->hasData(); }

size_t NetworkSerializer::writeRaw(const uint8* data, size_t size)
{
	PR_ASSERT(isValid(), "Trying to write into a close buffer!");
	PR_ASSERT(!isReadMode(), "Trying to write into a read serializer!");

	mSocket->send((const char*)data, size);
	return size;
}

size_t NetworkSerializer::readRaw(uint8* data, size_t size)
{
	PR_ASSERT(isValid(), "Trying to read from a close buffer!");
	PR_ASSERT(isReadMode(), "Trying to read from a write serializer!");

	bool ok;
	size_t read = mSocket->receiveAtMost((char*)data, size, &ok);
	if (!ok)
		PR_LOG(L_ERROR) << "Could not read from socket." << std::endl;
	else if (read == 0) // Closed
		mSocket = nullptr;

	return read;
}

} // namespace PR