#include "pch.h"
#include "SocketCallback.h"

uint32_t SocketCallback::OnClientAccept(SOCKET client, const char* addr, const uint16_t port)
{
	LOG(INFO) << "Accept from:[" << addr << ":" << port << "]";

	std::unique_lock<std::mutex> lock(client_fd_lock_);

	client_fd_list_.emplace_back(SocketInfo{ client, addr,port });

	cv_fd_list_.notify_all();

	return 0;
}
uint32_t SocketCallback::OnServerCreated()
{
	thread_pool::ThreadPool::instance().CommitTask(
		[self = shared_from_this()] 
		{
			self->StartSelect();
		}
	);
	return 0;
}

uint32_t SocketCallback::OnStop()
{
	stop_ = true;
	std::unique_lock<std::mutex> lock(client_fd_lock_);
	cv_fd_list_.notify_all();
	return 0;
}

void SocketCallback::StartSelect()
{
	LOG(INFO) << "Start select.";
	while (!stop_)
	{
		decltype(client_fd_list_)temp_list;
		{
			std::unique_lock<std::mutex> lock(client_fd_lock_);
			cv_fd_list_.wait(lock, [this]
				{
					return stop_ || !client_fd_list_.empty();
				});
			if (stop_)break;
			temp_list = client_fd_list_;
		}
		
		fd_set client_set;
		FD_ZERO(&client_set);
		SOCKET max_fd{ 0 };

		for (auto s : temp_list)
		{
			FD_SET(s, &client_set);
			max_fd = max(s, max_fd);
		}

		timeval val;
		val.tv_sec = 0;
		val.tv_usec = 100;
		const auto sres = select(max_fd + 1, &client_set, nullptr, nullptr, &val);
		if (sres < 0)break;
		if (sres == 0)continue;

		auto readnum{ 0 };
		for (auto s : temp_list)
		{
			static const int32_t buff_len{ 1025 };
			char buff[buff_len]{ 0 };
			if (FD_ISSET(s, &client_set))
			{
				auto read_len = recv(s, buff, buff_len-1, 0);
				if (read_len <= 0)
				{
					OnClientClosed(s);
					continue;
				}

				buff[read_len] = 0;
				LOG(INFO) << "Recive data from:[" << s.addr << "] port:[" << s.port << "] data:[" << buff << "]";
				send(s, "hello", strlen("hello"), 0);
			}
		}
	}

	std::unique_lock<std::mutex> lock(client_fd_lock_);
	for (auto s : client_fd_list_)
	{
		closesocket(s);
	}
}

void SocketCallback::OnClientClosed(SOCKET s)
{
	std::unique_lock<std::mutex> lock(client_fd_lock_);
	auto iter = std::find(client_fd_list_.begin(), client_fd_list_.end(), s);
	if (iter != client_fd_list_.end())
	{
		LOG(INFO) << "Client disconnect! from:[" << iter->addr << "] port:[" << iter->port << "]";
		client_fd_list_.erase(iter);
	}
	closesocket(s);
}
