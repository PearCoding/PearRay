#include "Socket.h"
#include "Logger.h"

#include <atomic>
#include <chrono>
#include <thread>

#include <signal.h>

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

	bool IsClientConnection = false;
	bool IsOpen				= false;
	bool IsIp6				= false;

	// Ip4
	inline bool connect4(uint16 port, const std::string& ip)
	{
		sockaddr_in addr;
		memset(&addr, 0, sizeof(addr));

		if (!getAddressFromString4(ip, addr)) {
			PR_LOG(L_ERROR) << "Couldn't convert ip address: " << getErrorString() << std::endl;
			return false;
		}
		addr.sin_family = AF_INET;
		addr.sin_port	= htons(port);

		int error = ::connect(Socket, (sockaddr*)&addr, sizeof(addr));

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

		int error = ::bind(Socket, (sockaddr*)&addr, sizeof(addr));

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
		IsOpen = true;
		return true;
	}

	// Ip6
	inline bool connect6(uint16 port, const std::string& ip)
	{
		sockaddr_in6 addr;
		memset(&addr, 0, sizeof(addr));

		if (!getAddressFromString6(ip, addr)) {
			PR_LOG(L_ERROR) << "Couldn't convert ip address: " << getErrorString() << std::endl;
			return false;
		}
		addr.sin6_family = AF_INET6;
		addr.sin6_port	 = htons(port);

		int error = ::connect(Socket, (sockaddr*)&addr, sizeof(addr));

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

		int error = ::bind(Socket, (sockaddr*)&addr, sizeof(addr));

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
		IsOpen = true;
		return true;
	}
};

Socket::Socket()
	: mInternal(new SocketInternal())
{
	static std::atomic<bool> sInitialized = false;
	if (!sInitialized.exchange(true)) {
		signal(SIGPIPE, SIG_IGN);
#ifdef PR_OS_WINDOWS
		initNetwork(); // We should close the network support but why bother?
#endif
	}

	socket_t socket;
#if !defined(PR_NO_IPV6)
	socket			 = ::socket(AF_INET6, SOCK_STREAM, 0);
	mInternal->IsIp6 = true;

	if (!checkSocketAndErrorCode(socket)) {
		PR_LOG(L_WARNING) << "No IPv6 support" << std::endl;
#endif

		socket = ::socket(AF_INET, SOCK_STREAM, 0);
		checkSocketAndErrorCode(socket);
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
	mInternal->IsClientConnection = true;
}

Socket::~Socket()
{
	if (mInternal)
		closeSocket(mInternal->Socket);
}

Socket::Socket(Socket&& other)
	: mInternal(std::move(other.mInternal))
{
}

Socket& Socket::operator=(Socket&& other)
{
	mInternal = std::move(other.mInternal);
	return *this;
}

bool Socket::connect(uint16 port, const std::string& ip)
{
	if (mInternal->IsIp6) {
		if (!mInternal->connect6(port, ip)) {
			PR_LOG(L_WARNING) << "No IPv6 support" << std::endl;
			return mInternal->connect4(port, ip);
		}
	} else {
		if (!mInternal->connect4(port, ip))
			return false;
	}

	mInternal->IsOpen = true;
	return true;
}

bool Socket::bindAndListen(uint16 port, int32 maxClients)
{
	int max = maxClients < 0 ? SOMAXCONN : maxClients;
	if (mInternal->IsIp6) {
		if (!mInternal->bind6(port)) {
			PR_LOG(L_WARNING) << "No IPv6 support" << std::endl;
			return mInternal->bind4(port);
		}
	} else {
		if (!mInternal->bind4(port))
			return false;
	}

	int error = ::listen(mInternal->Socket, max);

	if (isSocketError(error))
		return false;

	mInternal->IsOpen = true;
	return true;
}

Socket Socket::accept()
{
	Socket acceptedSocket(true);
	if (mInternal->IsIp6) {
		if (!acceptedSocket.mInternal->accept6(mInternal->Socket))
			acceptedSocket.mInternal->accept4(mInternal->Socket);
	} else {
		acceptedSocket.mInternal->accept4(mInternal->Socket);
	}
	return acceptedSocket;
}

bool Socket::hasData() const
{
	fd_set readableSet;
	FD_ZERO(&readableSet);
	FD_SET(mInternal->Socket, &readableSet);

	int error = ::select(mInternal->Socket + 1, &readableSet, nullptr, nullptr, nullptr);

	if (isSocketError(error))
		return false;

	return FD_ISSET(mInternal->Socket, &readableSet);
}

bool Socket::hasData(float timeout_s) const
{
	fd_set readableSet;
	FD_ZERO(&readableSet);
	FD_SET(mInternal->Socket, &readableSet);

	timeval tout;
	tout.tv_sec	 = (int)timeout_s;
	tout.tv_usec = (timeout_s - (int)timeout_s) * 1000000;

	int error = ::select(mInternal->Socket + 1, &readableSet, nullptr, nullptr, &tout);

	if (isSocketError(error))
		return false;

	return error > 0 && FD_ISSET(mInternal->Socket, &readableSet);
}

bool Socket::send(const char* data, size_t len)
{
	size_t total = 0;

	while (total < len) {
		int ret = ::send(mInternal->Socket, data + total, len - total, MSG_NOSIGNAL);

		if (isSocketError(ret)) {
			mInternal->IsOpen = false;
			return false;
		} else if (ret == 0) // Give other side time to catch up
			std::this_thread::sleep_for(std::chrono::milliseconds(5));

		total += ret;
	}

	return true;
}

bool Socket::receive(char* data, size_t len)
{
	size_t total = 0;

	while (total < len) {
		int ret = ::recv(mInternal->Socket, data, len, 0);
		if (isSocketError(ret)) {
			mInternal->IsOpen = false;
			return false;
		} else if (ret == 0) {
			mInternal->IsOpen = false;
			return false; // Really false?
		}

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
		mInternal->IsOpen = false;
		return 0;
	}

	if (ret == 0)
		mInternal->IsOpen = false;

	if (ok)
		*ok = true;
	return (size_t)ret;
}

bool Socket::isValid() const { return checkSocket(mInternal->Socket); }
bool Socket::isClientConnection() const { return mInternal->IsClientConnection; }
bool Socket::isOpen() const { return mInternal->IsOpen; }

std::string Socket::ip() const { return mInternal->IP; }
uint16 Socket::port() const { return mInternal->Port; }
} // namespace PR