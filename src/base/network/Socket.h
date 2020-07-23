#pragma once

#include "PR_Config.h"

namespace PR {
class PR_LIB_BASE Socket {
public:
	Socket();
	virtual ~Socket();

	Socket(Socket&& other) = default;
	Socket& operator=(Socket&& other) = default;

	// Client
	bool connect(uint16 port, const std::string& ip);

	// Server
	bool bind(uint16 port);
	bool listen(int32 maxClients = -1);
	Socket accept();

	// IO
	bool send(const char* data, size_t len);
	bool receive(char* data, size_t len);
	size_t receiveAtMost(char* data, size_t len, bool* ok);

	// Properties
	bool isValid() const;
	bool isListening() const;
	bool isConnection() const;

	std::string ip() const;
	uint16 port() const;

private:
	Socket(bool accept);
	std::unique_ptr<class SocketInternal> mInternal;
};
} // namespace PR