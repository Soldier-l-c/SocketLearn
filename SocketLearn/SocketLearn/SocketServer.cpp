#include "pch.h"
#include "SocketServer.h"
#define BACKLOG 13

bool SocketServer::CreateServer(const std::string& addr, uint16_t port, ISocketCallbackPtr call_back)
{
	call_back_ = call_back;
	SOCKET listen_fd{ 0 };
	if (!ServerInit(addr, port, &listen_fd) || listen_fd<0 )
	{
		return false;
	}

	listen_socket_ = listen_fd;
	call_back->OnServerCreated();

	Listen(listen_fd);

	Stop();

	return true;
}

void SocketServer::Stop()
{
	if (stop_)return;

	stop_ = true;
	if (listen_socket_ >= 0)
	{
		closesocket(listen_socket_);
		listen_socket_ = 0;
	}

	if (call_back_)
	{
		call_back_->OnStop();
	}
}

bool SocketServer::ServerInit(const std::string& addr, uint16_t port, SOCKET* listen_fd)
{
	auto listen_s = socket(AF_INET, SOCK_STREAM, 0);
	if (listen_s < 0)return false;

	char on = 1;
	if ((setsockopt(listen_s, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on))) < 0) /*让端口号能够立即重复使用*/
	{
		return false;
	}

	struct sockaddr_in servaddr;
	memset(&servaddr, 0, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(port);
	if (addr.empty())/*加入传入IP地址则监听指定ip，否则监听所有ip*/
	{
		servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	}
	else
	{
		servaddr.sin_addr.s_addr = inet_addr(addr.c_str());
	}

	if (bind(listen_s, (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0)/*绑定端口号和ip*/
	{
		return false;
	}

	if (listen_fd)
	{
		*listen_fd = listen_s;
	}
	return true;
}

bool SocketServer::Listen(SOCKET listen_fd)
{
	auto res = listen(listen_fd, BACKLOG);
	if (res < 0)return false;
	using socklen_t = int32_t;

	while (!stop_)
	{
		struct sockaddr_in cli_addr;
		memset(&cli_addr, 0, sizeof(cli_addr));
		socklen_t cliaddr_len{ sizeof(cli_addr) };
		int client_fd = accept(listen_fd, (sockaddr*)&cli_addr, &cliaddr_len);
		if (client_fd <= 0)break;

		if (call_back_)
		{
			call_back_->OnClientAccept(client_fd, inet_ntoa(cli_addr.sin_addr), ntohs(cli_addr.sin_port));
		}
	}

	return true;
}
