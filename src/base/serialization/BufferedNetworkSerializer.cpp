#include "BufferedNetworkSerializer.h"

namespace PR {
BufferedNetworkSerializer::BufferedNetworkSerializer()
	: BufferSerializer()
	, mNetwork()
{
}

BufferedNetworkSerializer::BufferedNetworkSerializer(Socket* socket, bool readmode, size_t bufferSize)
	: BufferSerializer()
	, mNetwork(socket, readmode)
{
	this->reset(&mNetwork, bufferSize);
}

BufferedNetworkSerializer::~BufferedNetworkSerializer()
{
	this->flush();
}

} // namespace PR