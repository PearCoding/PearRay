#pragma once

#include "PR_Config.h"

namespace PR {
class PR_LIB_BASE Socket {
public:
	Socket();
	virtual ~Socket();

	Socket(Socket&& other) noexcept;
	Socket& operator=(Socket&& other) noexcept;

	// Client
	bool connect(uint16 port, const std::string& ip);

	// Server
	bool bindAndListen(uint16 port, int32 maxClients = -1);
	Socket accept();

	bool hasData() const;
	bool hasData(float timeout_s) const;
	inline bool hasIncomingConnection(float timeout_s) const { return hasData(timeout_s); }

	// IO
	bool send(const char* data, size_t len);
	bool receive(char* data, size_t len);
	size_t receiveAtMost(char* data, size_t len, bool* ok);

	// Properties
	bool isValid() const;
	bool isClientConnection() const;
	bool isOpen() const;

	std::string ip() const;
	uint16 port() const;

private:
	Socket(bool accept);
	std::unique_ptr<class SocketInternal> mInternal;
};
} // namespace PR