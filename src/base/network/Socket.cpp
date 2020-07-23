#include "Socket.h"
#include "Logger.h"

#ifdef PR_OS_WINDOWS
#include "SocketImplWindows.inl"
#else
#include "SocketImplLinux.inl"
#endif

namespace PR {

class SocketInternal {
public:
	socket_t Socket = -1;
	std::string IP	= "127.0.0.1";
	uint16 Port		= 0;

	bool IsListening  = false;
	bool IsConnection = false;
	bool IsIp6		  = false;

	// Ip4
	inline bool connect4(uint16 port, const std::string& ip)
	{
		sockaddr_in addr;
		memset(&addr, 0, sizeof(addr));

		addr.sin_family = AF_INET;
		addr.sin_port	= htons(port);
		if (!getAddressFromString4(ip, addr)) {
			PR_LOG(L_ERROR) << "Couldn't convert ip address: " << getErrorString() << std::endl;
			return false;
		}

		int error = ::connect(Socket, (sockaddr*)&addr, sizeof(sockaddr));

		if (isSocketError(error))
			return false;

		IP	 = ip;
		Port = port;
		return true;
	}

	inline bool bind4(uint16 port)
	{
		sockaddr_in addr;
		memset(&addr, 0, sizeof(addr));

		addr.sin_family		 = AF_INET;
		addr.sin_port		 = htons(port);
		addr.sin_addr.s_addr = INADDR_ANY;

		int error = ::bind(Socket, (sockaddr*)&addr, sizeof(sockaddr));

		if (isSocketError(error))
			return false;

		Port = port;
		return true;
	}

	inline bool accept4(socket_t server)
	{
		sockaddr_in addr;
		memset(&addr, 0, sizeof(addr));
		socklen_t size = sizeof(addr);

		socket_t newsocket = ::accept(server, (sockaddr*)&addr, &size);

		if (checkSocketAndErrorCode(newsocket))
			return false;

		Socket = newsocket;
		IP	   = getStringFromAddress4(addr);
		Port   = ntohs(addr.sin_port);
		IsIp6  = false;
		return true;
	}

	// Ip6
	inline bool connect6(uint16 port, const std::string& ip)
	{
		sockaddr_in6 addr;
		memset(&addr, 0, sizeof(addr));

		addr.sin6_family = AF_INET;
		addr.sin6_port	 = htons(port);
		if (!getAddressFromString6(ip, addr)) {
			PR_LOG(L_ERROR) << "Couldn't convert ip address: " << getErrorString() << std::endl;
			return false;
		}

		int error = ::connect(Socket, (sockaddr*)&addr, sizeof(sockaddr));

		if (isSocketError(error))
			return false;

		IP	 = ip;
		Port = port;
		return true;
	}

	inline bool bind6(uint16 port)
	{
		sockaddr_in6 addr;
		memset(&addr, 0, sizeof(addr));

		addr.sin6_family = AF_INET6;
		addr.sin6_port	 = htons(port);
		addr.sin6_addr	 = in6addr_any;

		int error = ::bind(Socket, (sockaddr*)&addr, sizeof(sockaddr));

		if (isSocketError(error))
			return false;

		Port = port;
		return true;
	}

	inline bool accept6(socket_t server)
	{
		sockaddr_in6 addr;
		memset(&addr, 0, sizeof(addr));
		socklen_t size = sizeof(addr);

		socket_t newsocket = ::accept(server, (sockaddr*)&addr, &size);

		if (checkSocketAndErrorCode(newsocket))
			return false;

		Socket = newsocket;
		IP	   = getStringFromAddress6(addr);
		Port   = ntohs(addr.sin6_port);
		IsIp6  = true;
		return true;
	}
};

Socket::Socket()
	: mInternal(new SocketInternal())
{
#ifdef PR_OS_WINDOWS
	static std::atomic<bool> sInitialized = false;
	if (!sInitialized.exchange(true))
		initNetwork(); // We should close the network support but why bother?
#endif

	socket_t socket;
#if !defined(PR_NO_IPV6)
	socket			 = ::socket(AF_INET6, SOCK_STREAM, 0);
	mInternal->IsIp6 = true;

	if (!checkSocketAndErrorCode(socket)) {
		PR_LOG(L_WARNING) << "No IPv6 support" << std::endl;
#endif

		socket = ::socket(AF_INET, SOCK_STREAM, 0);
		checkSocket(socket);
		mInternal->IsIp6 = false;
#if !defined(PR_NO_IPV6)
	}
#endif
	mInternal->Socket = socket;
}

Socket::Socket(bool)
	: mInternal(new SocketInternal())
{
	// Do nothing
	mInternal->IsConnection = true;
}

Socket::~Socket()
{
	closeSocket(mInternal->Socket);
}

bool Socket::connect(uint16 port, const std::string& ip)
{
	if (mInternal->IsIp6) {
		if (!mInternal->connect6(port, ip)) {
			PR_LOG(L_WARNING) << "No IPv6 support" << std::endl;
			return mInternal->connect4(port, ip);
		}
		return true;
	} else {
		return mInternal->connect4(port, ip);
	}
}

bool Socket::bind(uint16 port)
{
	if (mInternal->IsIp6) {
		if (!mInternal->bind6(port)) {
			PR_LOG(L_WARNING) << "No IPv6 support" << std::endl;
			return mInternal->bind4(port);
		}
		return true;
	} else {
		return mInternal->bind4(port);
	}
}

bool Socket::listen(int32 maxClients)
{
	int max = maxClients < 0 ? SOMAXCONN : maxClients;

	int error = ::listen(mInternal->Socket, max);

	if (isSocketError(error))
		return false;

	mInternal->IsListening = true;
	return true;
}

Socket Socket::accept()
{
	PR_ASSERT(mInternal->IsListening, "Trying to accept on a non-listening socket");
	Socket acceptedSocket(true);
	if (mInternal->IsIp6) {
		if (!acceptedSocket.mInternal->accept6(mInternal->Socket))
			acceptedSocket.mInternal->accept4(mInternal->Socket);
	} else {
		acceptedSocket.mInternal->accept4(mInternal->Socket);
	}
	return acceptedSocket;
}

bool Socket::send(const char* data, size_t len)
{
	size_t total = 0;

	while (total < len) {
		int ret = ::send(mInternal->Socket, data + total, len - total, 0);

		if (isSocketError(ret))
			return false;

		total += ret;
	}

	return true;
}

bool Socket::receive(char* data, size_t len)
{
	size_t total = 0;

	while (total < len) {
		int ret = ::recv(mInternal->Socket, data, len, 0);
		if (isSocketError(ret))
			return false;

		total += ret;
	}

	return true;
}

size_t Socket::receiveAtMost(char* data, size_t len, bool* ok)
{
	int ret = ::recv(mInternal->Socket, data, len, 0);
	if (isSocketError(ret)) {
		if (ok)
			*ok = false;
		return 0;
	}

	if (ok)
		*ok = true;
	return (size_t)ret;
}

bool Socket::isValid() const { return checkSocket(mInternal->Socket); }
bool Socket::isListening() const { return mInternal->IsListening; }
bool Socket::isConnection() const { return mInternal->IsConnection; }

std::string Socket::ip() const { return mInternal->IP; }
uint16 Socket::port() const { return mInternal->Port; }
} // namespace PR