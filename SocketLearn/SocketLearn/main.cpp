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


class Solution
{
public:

    using PointTag = std::pair<std::vector<int>, char>;

    struct Cmp
    {
        bool operator()(const PointTag& left, const PointTag& right)const
        {
            return std::max(abs(left.first[0]), abs(left.first[1])) < std::max(abs(right.first[0]), abs(right.first[1]));
        }
    };

    int maxPointsInsideSquare(std::vector<std::vector<int>>& points, std::string s)
    {
        std::vector<bool>visit(26, false);
        std::vector<PointTag> point_tags;
        int res{ 0 };
        for (int i = 0; i < points.size(); ++i)
        {
            point_tags.emplace_back(points[i], s[i]);
        }
        std::less<int >;
        std::sort(point_tags.begin(), point_tags.end(), Cmp());

        for (auto i = 0; i < points.size(); ++i)
        {
            if (visit[point_tags[i].second])
            {
                return point_tags[i] == point_tags[i - 1] ? res - 1 : res;
            }
            visit[point_tags[i].second] = true;
            res++;
        }
        return  res;
    }

    int maxPointsInsideSquare1(std::vector<std::vector<int>>& points, std::string s) 
    {
        int ans = 0;
        auto check = [&](int size) -> bool {
            int vis = 0;
            for (int i = 0; i < points.size(); i++) 
            {
                if (abs(points[i][0]) <= size && abs(points[i][1]) <= size) 
                {
                    char c = s[i] - 'a';
                    if (vis >> c & 1) 
                    {
                        return false;
                    }
                    vis |= 1 << c;
                }
            }
            ans = __builtin_popcount(vis);
            return true;
        };
        int left = -1, right = 1'000'000'001;
        while (left + 1 < right) 
        {
            int mid = (left + right) / 2;
            (check(mid) ? left : right) = mid;
        }
        return ans;
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

    auto res1 = thread_pool::ThreadPool::instance().CommitTask([&server]
        {
            httplib::Server s;
            
            s.Get("/hello", [](const httplib::Request& q, httplib::Response& r) 
                {
                    r.set_content("hello", "text");
                    
                });
            s.listen("127.0.0.1", 80);
        });

    res.get();

	LOG(INFO) << "Server Stop.";

    system("pause");

    return 0;
}