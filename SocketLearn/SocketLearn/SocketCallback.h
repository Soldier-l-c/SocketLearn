#pragma once
#include "ISocketCallback.h"

struct SocketInfo
{
	SOCKET sk{ 0 };
	std::string addr;
	uint16_t port{ 0 };
	operator SOCKET() { return sk; };

	SocketInfo(SOCKET s, const char* addr, const uint16_t port) : sk(s), addr(addr), port(port)
	{};

	bool operator == (const SocketInfo& s)
	{
		return s.sk == sk;
	}
	bool operator == (const SOCKET& s)
	{
		return s == sk;
	}
};

class SocketCallback :public ISocketCallback, public std::enable_shared_from_this<SocketCallback>
{
public:
	virtual uint32_t OnClientAccept(SOCKET client, const char* addr, const uint16_t port);

	virtual uint32_t OnServerCreated();

	virtual uint32_t OnStop();

	void StartSelect();

private:

	void OnClientClosed(SOCKET s);

public:
	std::mutex client_fd_lock_;
	std::condition_variable cv_fd_list_;
	std::vector<SocketInfo> client_fd_list_;
	std::atomic_bool stop_{ false };
};