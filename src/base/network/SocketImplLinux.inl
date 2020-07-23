#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

namespace PR {
using socket_t			   = int;
constexpr int SOCKET_ERROR = -1;

inline const char* getErrorString()
{
	return ::strerror(errno);
}

inline void printError()
{
	PR_LOG(L_ERROR) << "[Network] " << getErrorString() << std::endl;
}

inline bool isSocketError(int error)
{
	if (error == SOCKET_ERROR) {
		printError();
		return true;
	}

	return false;
}

inline bool checkSocketAndErrorCode(socket_t socket)
{
	if (socket == SOCKET_ERROR) {
		printError();
		return true;
	}

	return false;
}

inline bool checkSocket(socket_t socket)
{
	return socket != SOCKET_ERROR;
}

inline std::string getStringFromAddress4(sockaddr_in& addr)
{
	thread_local char buffer[INET_ADDRSTRLEN];
	const char* str = inet_ntop(AF_INET, &addr.sin_addr, buffer, INET_ADDRSTRLEN);
	if (!str) {
		printError();
		return "";
	} else {
		return str;
	}
}

inline std::string getStringFromAddress6(sockaddr_in6& addr)
{
	thread_local char buffer[INET6_ADDRSTRLEN];
	const char* str = inet_ntop(AF_INET6, &addr.sin6_addr, buffer, INET6_ADDRSTRLEN);
	if (!str) {
		printError();
		return "";
	} else {
		return str;
	}
}

inline bool getAddressFromString4(const std::string& ip, sockaddr_in& addr)
{
	addrinfo* _addrinfo;
	bool good	= false;
	int errcode = ::getaddrinfo(ip.c_str(), nullptr, nullptr, &_addrinfo);
	if (errcode != 0) {
		PR_LOG(L_ERROR) << "[Network] " << gai_strerror(errcode) << std::endl;
		return good;
	} else {

		for (auto it = _addrinfo; it; it = it->ai_next) {
			if (it->ai_family == AF_INET) {
				addr = *(sockaddr_in*)it->ai_addr;
				good = true;
				break;
			}
		}
	}

	freeaddrinfo(_addrinfo);
	return good;
}

inline bool getAddressFromString6(const std::string& ip, sockaddr_in6& addr)
{
	addrinfo* _addrinfo;
	bool good	= false;
	int errcode = ::getaddrinfo(ip.c_str(), nullptr, nullptr, &_addrinfo);
	if (errcode != 0) {
		PR_LOG(L_ERROR) << "[Network] " << gai_strerror(errcode) << std::endl;
		return good;
	} else {

		for (auto it = _addrinfo; it; it = it->ai_next) {
			if (it->ai_family == AF_INET6) {
				addr = *(sockaddr_in6*)it->ai_addr;
				good = true;
				break;
			}
		}
	}

	freeaddrinfo(_addrinfo);
	return good;
}

inline void closeSocket(socket_t socket)
{
	::close(socket);
}

} // namespace PR