#pragma once
struct ISocketCallback
{
	virtual uint32_t OnClientAccept(SOCKET client, const char* addr, const uint16_t port) = 0;
	virtual uint32_t OnServerCreated() = 0;
	virtual uint32_t OnStop() = 0;
};

using ISocketCallbackPtr = std::shared_ptr<ISocketCallback>;
