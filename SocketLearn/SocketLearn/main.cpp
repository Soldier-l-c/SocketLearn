#include "pch.h"
#include "SocketServer.h"
#include "SocketCallback.h"

void InitLogger()
{
	static std::once_flag flag;
	std::call_once(flag, [] 
		{
			g_logger = std::make_shared<console_logger::InternalLogger>();
			g_logger->Init(L"");
		});
}

class WSASocketHelper
{
public:
    WSASocketHelper()
    {
        WSADATA wsaData;
        auto err = WSAStartup(MAKEWORD(2, 2), &wsaData);

        if (err != 0) {
            LOG(ERROR) << L"Init wsa failed error code:[" << err << "]";
            return;
        }

        if (LOBYTE(wsaData.wVersion) != 2 ||
            HIBYTE(wsaData.wHighVersion) != 2) {
            WSACleanup();
            LOG(ERROR) << L"Init wsa failed";
            return;
        }
    }
    ~WSASocketHelper()
    {
        WSACleanup();
    }
};

int main()
{
	InitLogger();

	WSASocketHelper wsa_helper;

	LOG(INFO) << "Server Start.";
	
    SocketServer server;

    auto res = thread_pool::ThreadPool::instance().CommitTask([&server] 
        {
            return server.CreateServer("127.0.0.1", 8888, std::make_shared<SocketCallback>());
        });

    res.get();

	LOG(INFO) << "Server Stop.";

    system("pause");

    return 0;
}