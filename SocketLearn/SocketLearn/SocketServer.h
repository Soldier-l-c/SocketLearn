#pragma once
#include "ISocketCallback.h"

class SocketServer
{
public:
	bool CreateServer(const std::string& addr, uint16_t port, ISocketCallbackPtr call_back);
	void Stop();

private:
	bool ServerInit(const std::string& addr, uint16_t port, SOCKET* listen_fd);
	bool Listen(SOCKET listen_fd);

private:
	ISocketCallbackPtr call_back_;
	std::atomic_bool stop_{ false };
	std::atomic_int32_t listen_socket_{ -1 };
};

