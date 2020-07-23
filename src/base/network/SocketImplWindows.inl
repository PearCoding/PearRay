#include <winsock2.h>
#include <ws2tcpip.h>

// TODO: This is not finished. Implementation is incomplete
namespace PR {
using socket_t = int;

/* The error string is the same in http://msdn.microsoft.com/library/ms740668.aspx
 * So more information about the error could you find in this very nice page :D
 */
const char* getErrorString()
{
	switch (WSAGetLastError()) {
	case WSA_INVALID_HANDLE:
		return "Specified event object handle is invalid";
	case WSA_NOT_ENOUGH_MEMORY:
		return "Insufficient memory available";
	case WSA_INVALID_PARAMETER:
		return "One or more parameters are invalid";
	case WSA_OPERATION_ABORTED:
		return "Overlapped operation aborted";
	case WSA_IO_INCOMPLETE:
		return "Overlapped I/O event object not in signaled state";
	case WSA_IO_PENDING:
		return "Overlapped operations will complete later";
	case WSAEINTR:
		return "Interrupted function call";
	case WSAEBADF:
		return "File handle is not valid";
	case WSAEACCES:
		return "Permission denied";
	case WSAEFAULT:
		return "Bad address";
	case WSAEINVAL:
		return "Invalid argument";
	case WSAEMFILE:
		return "Too many open files";
	case WSAEWOULDBLOCK:
		return "Resource temporarily unavailable";
	case WSAEINPROGRESS:
		return "Operation now in progress";
	case WSAEALREADY:
		return "Operation already in progress";
	case WSAENOTSOCK:
		return "Socket operation on nonsocket";
	case WSAEDESTADDRREQ:
		return "Destination address required";
	case WSAEMSGSIZE:
		return "Message too long";
	case WSAEPROTOTYPE:
		return "Protocol wrong type for socket";
	case WSAENOPROTOOPT:
		return "Bad protocol option";
	case WSAEPROTONOSUPPORT:
		return "Protocol not supported";
	case WSAESOCKTNOSUPPORT:
		return "Socket type not supported";
	case WSAEOPNOTSUPP:
		return "Operation not supported";
	case WSAEPFNOSUPPORT:
		return "Protocol family not supported";
	case WSAEAFNOSUPPORT:
		return "Address family not supported by protocol family";
	case WSAEADDRINUSE:
		return "Address already in use";
	case WSAEADDRNOTAVAIL:
		return "Cannot assign requested address";
	case WSAENETDOWN:
		return "Network is down";
	case WSAENETUNREACH:
		return "Network is unreachable";
	case WSAENETRESET:
		return "Network dropped connection on reset";
	case WSAECONNABORTED:
		return "Software caused connection abort";
	case WSAECONNRESET:
		return "Connection reset by peer";
	case WSAENOBUFS:
		return "No buffer space available";
	case WSAEISCONN:
		return "Socket is already connected";
	case WSAENOTCONN:
		return "Socket is not connected";
	case WSAESHUTDOWN:
		return "Cannot send after socket shutdown";
	case WSAETOOMANYREFS:
		return "Too many references";
	case WSAETIMEDOUT:
		return "Connection timed out";
	case WSAECONNREFUSED:
		return "Connection refused";
	case WSAELOOP:
		return "Cannot translate name";
	case WSAENAMETOOLONG:
		return "Name too long";
	case WSAEHOSTDOWN:
		return "Host is down";
	case WSAEHOSTUNREACH:
		return "No route to host";
	case WSAENOTEMPTY:
		return "Directory not empty";
	case WSAEPROCLIM:
		return "Too many processes";
	case WSAEUSERS:
		return "User quota exceeded";
	case WSAEDQUOT:
		return "Disk quota exceeded";
	case WSAESTALE:
		return "Stale file handle reference";
	case WSAEREMOTE:
		return "Item is remote";
	case WSASYSNOTREADY:
		return "Network subsystem is unavailable";
	case WSAVERNOTSUPPORTED:
		return "Winsock.dll version out of range";
	case WSANOTINITIALISED:
		return "Successful WSAStartup not yet performed";
	case WSAEDISCON:
		return "Graceful shutdown in progress";
	case WSAENOMORE:
		return "No more results";
	case WSAECANCELLED:
		return "Call has been canceled";
	case WSAEINVALIDPROCTABLE:
		return "Procedure call table is invalid";
	case WSAEINVALIDPROVIDER:
		return "Service provider is invalid";
	case WSAEPROVIDERFAILEDINIT:
		return "Service provider failed to initialize";
	case WSASYSCALLFAILURE:
		return "System call failure";
	case WSASERVICE_NOT_FOUND:
		return "Service not found";
	case WSATYPE_NOT_FOUND:
		return "Class type not found";
	case WSA_E_NO_MORE:
		return "No more results";
	case WSA_E_CANCELLED:
		return "Call was canceled";
	case WSAEREFUSED:
		return "Database query was refused";
	case WSAHOST_NOT_FOUND:
		return "Host not found";
	case WSATRY_AGAIN:
		return "Nonauthoritative host not found";
	case WSANO_RECOVERY:
		return "This is a nonrecoverable error";
	case WSANO_DATA:
		return "Valid name, no data record of requested type";
	case WSA_QOS_RECEIVERS:
		return "QOS receivers";
	case WSA_QOS_SENDERS:
		return "QOS senders";
	case WSA_QOS_NO_SENDERS:
		return "No QOS senders";
	case WSA_QOS_NO_RECEIVERS:
		return "QOS no receivers";
	case WSA_QOS_REQUEST_CONFIRMED:
		return "QOS request confirmed";
	case WSA_QOS_ADMISSION_FAILURE:
		return "QOS admission error";
	case WSA_QOS_POLICY_FAILURE:
		return "QOS policy failure";
	case WSA_QOS_BAD_STYLE:
		return "QOS bad style";
	case WSA_QOS_BAD_OBJECT:
		return "QOS bad object";
	case WSA_QOS_TRAFFIC_CTRL_ERROR:
		return "QOS traffic control error";
	case WSA_QOS_GENERIC_ERROR:
		return "QOS generic error";
	case WSA_QOS_ESERVICETYPE:
		return "QOS service type error";
	case WSA_QOS_EFLOWSPEC:
		return "QOS flowspec error";
	case WSA_QOS_EPROVSPECBUF:
		return "Invalid QOS provider buffer";
	case WSA_QOS_EFILTERSTYLE:
		return "Invalid QOS filter style";
	case WSA_QOS_EFILTERTYPE:
		return "Invalid QOS filter type";
	case WSA_QOS_EFILTERCOUNT:
		return "Incorrect QOS filter count";
	case WSA_QOS_EOBJLENGTH:
		return "Invalid QOS object length";
	case WSA_QOS_EFLOWCOUNT:
		return "Incorrect QOS flow count";
	case WSA_QOS_EUNKOWNPSOBJ:
		return "Unrecognized QOS object";
	case WSA_QOS_EPOLICYOBJ:
		return "Invalid QOS policy object";
	case WSA_QOS_EFLOWDESC:
		return "Invalid QOS flow descriptor";
	case WSA_QOS_EPSFLOWSPEC:
		return "Invalid QOS provider-specific flowspec";
	case WSA_QOS_EPSFILTERSPEC:
		return "Invalid QOS provider-specific filterspec";
	case WSA_QOS_ESDMODEOBJ:
		return "Invalid QOS shape discard mode object";
	case WSA_QOS_ESHAPERATEOBJ:
		return "Invalid QOS shaping rate object";
	case WSA_QOS_RESERVED_PETYPE:
		return "Reserved policy QOS element type";
	default:
		return "Unknown error occured";
	}
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
	if (socket == INVALID_SOCKET) {
		printError();
		return true;
	}

	return false;
}

inline bool checkSocket(socket_t socket)
{
	return socket != INVALID_SOCKET;
}

inline std::string getStringFromAddress(SOCKADDR_IN& addr)
{
	DWORD bufferSize = 256;
	thread_local char buffer[256];

	int error = WSAAddressToString((SOCKADDR*)&addr, sizeof(SOCKADDR_IN), 0, buffer, &bufferSize);

	if (checkSocketError(error) || bufferSize <= 0)
		return "";

	return buffer;
}

inline void closeSocket(socket_t socket)
{
	::closesocket(socket);
}

inline bool initNetwork()
{
	WORD version;
	WSADATA wsa;

	version	  = MAKEWORD(2, 2);
	int error = WSAStartup(version, &wsa);

	if (error) {
		PR_LOG(L_ERROR) << "[Network] Couldn't init WinSock2: " << getErrorString(WSAGetLastError()) << std::endl;
		return false;
	}

	if (LOBYTE(wsa.wVersion) != 2 || HIBYTE(wsa.wVersion) != 2) {
		PR_LOG(L_ERROR) << "[Network] Couldn't find a usable version of Winsock.dll" << std::endl;
		WSACleanup();
		return false;
	}

	return true;
}

inline void closeNetwork()
{
	WSACleanup();
}

} // namespace PR