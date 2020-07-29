#include "BufferedNetworkSerializer.h"

namespace PR {
BufferedNetworkSerializer::BufferedNetworkSerializer()
	: BufferSerializer()
	, mNetwork()
{
	this->reset(&mNetwork, 4096);
}

BufferedNetworkSerializer::BufferedNetworkSerializer(Socket* socket, bool readmode, size_t bufferSize)
	: BufferSerializer()
	, mNetwork(socket, readmode)
{
	this->reset(&mNetwork, bufferSize);
}

BufferedNetworkSerializer::~BufferedNetworkSerializer()
{
	if (isValid())
		this->flush();
}

void BufferedNetworkSerializer::setSocket(Socket* socket, bool readmode)
{
	mNetwork.setSocket(socket, readmode);
	setReadMode(readmode);
}

} // namespace PR