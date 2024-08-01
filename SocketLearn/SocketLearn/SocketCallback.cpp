#include "pch.h"
#include "SocketCallback.h"

uint32_t SocketCallback::OnClientAccept(SOCKET client, const char* addr, const uint16_t port)
{
	LOG(INFO) << "Accept from:[" << addr << ":" << port << "] client fd:[" << client << "]";

	std::unique_lock<std::mutex> lock(client_fd_lock_);

	client_fd_list_.emplace_back(std::make_shared<SocketInfo>( client, addr,port));

	cv_fd_list_.notify_all();

	return 0;
}
uint32_t SocketCallback::OnServerCreated()
{
	future_ = thread_pool::ThreadPool::instance().CommitTask(
		[self = shared_from_this()] 
		{
			self->StartSelect();
		}
	);
	return 0;
}

uint32_t SocketCallback::OnStop()
{
	if (stop_)return 0;

	stop_ = true;
	
	{
		std::unique_lock<std::mutex> lock(client_fd_lock_);
		cv_fd_list_.notify_all();
	}

	future_.get();

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

		if (!Select(temp_list))break;
	}

	std::unique_lock<std::mutex> lock(client_fd_lock_);
	for (const auto& s : client_fd_list_)
	{
		closesocket(s->sk);
	}
}

bool SocketCallback::Select(const SocketInfoList& socket_list)
{
	fd_set client_set;
	FD_ZERO(&client_set);
	SOCKET max_fd{ 0 };

	for (const auto& s : socket_list)
	{
		FD_SET(s->sk, &client_set);
		max_fd = max(s->sk, max_fd);
	}

	timeval val;
	val.tv_sec = 0;
	val.tv_usec = 100;
	const auto sres = select(max_fd + 1, &client_set, nullptr, nullptr, &val);
	if (sres < 0)return false;
	if (sres == 0)return true;

	auto readnum{ 0 };
	for (const auto& s : socket_list)
	{
		static const int32_t buff_len{ 10 };
		char buff[buff_len]{ 0 };
		if (FD_ISSET(s->sk, &client_set))
		{
			auto read_len = recv(s->sk, buff, buff_len - 1, 0);
			if (read_len <= 0)
			{
				OnClientClosed(s->sk);
				continue;
			}

			buff[read_len] = 0;
			LOG(INFO) << "Recive data from:[" << s->addr << "] port:[" << s->port << "] data:[" <<  buff << "]";
			send(s->sk, "hello", strlen("hello"), 0);
		}
	}

	return true;
}

void SocketCallback::OnClientClosed(SOCKET s)
{
	std::unique_lock<std::mutex> lock(client_fd_lock_);
	auto iter = std::find_if(client_fd_list_.begin(), client_fd_list_.end(), [s] (const SocketInfoPtr& info)
		{
			return info->sk == s;
		});
	if (iter != client_fd_list_.end())
	{
		LOG(INFO) << "Client disconnect! from:[" << (*iter)->addr << "] port:[" << (*iter)->port << "]";
		client_fd_list_.erase(iter);
	}
	closesocket(s);
}
