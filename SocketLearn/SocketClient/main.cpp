#include "pch.h"

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
            LOG(ERROR)<<L"Init wsa failed error code:["<<err<<"]";
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


int test()
{
    int conn_fd = -1;
    int rv = -1;
    char buf[1024];
    struct sockaddr_in serv_addr;

    conn_fd = socket(AF_INET, SOCK_STREAM, 0);/*socket创建客户端的描述符*/
    if (conn_fd < 0)
    {
        printf("create client socket failure:%d\n", (errno));
        return -1;
    }

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(8888);
    serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");/*将点分十进制转换成32位整型类型*/

    if (connect(conn_fd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0)/*连接服务器*/
    {
        printf("client[%d] connect to server[%s:%d] failure:%d\n", conn_fd, "127.0.0.1", 8888, (errno));
        return -1;
    }

    auto data = GetFormatTime();
    if (send(conn_fd, data.c_str(), data.length(), 0) < 0)
    {
        printf("write data to server[%s,%d] failure:%d\n", "127.0.0.1", 8888, (errno));
        return -2;
    }

    memset(buf, 0, sizeof(buf));
    rv = recv(conn_fd, buf, sizeof(buf), 0);
    if (rv < 0)
    {
        printf("read data from server failure:%d\n", (errno));
        return -3;
    }
    else if (rv == 0)
    {
        printf("client connetc to server get disconnect\n");
        return -4;
    }
    printf("read %d bytes from server:'%s'\n", rv, buf);

    return 0;
}

int main()
{
	InitLogger();
    WSASocketHelper wsa_helper;
    test();
}