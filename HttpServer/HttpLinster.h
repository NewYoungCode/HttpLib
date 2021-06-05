#pragma once
#include <string>
#include <functional>
#include <map>
#include <vector>
#include <regex>
#include <memory>

#include "Socket.hpp"
#include "String.hpp"
#include "FileSystem.hpp"
#include "ThreadPool.hpp"

namespace HttpLib {
	//忽略控制台输出
	template<class ...Args>
	void printf(const char* str, Args... args) {
		//std::printf(str, std::forward<Args>(args)...);
	}
#define GET 0
#define POST 1
	struct Form {
		size_t ContentLength;
		size_t Read(char* buf, size_t count);
		size_t DataCount;
		String Filed;
		char* DataPos = NULL;
	};
	//请求结构体
	struct Request
	{
	private:
		size_t ReadCount = 0;
	public:
		size_t HeadPos = std::string::npos;
		String Temp;
		String Cookie;
		String	RawUrl;
		String Url;
		String Header;
		std::vector<Form> Forms;
		std::map<String, String> Headers;
		char Method = GET;
		std::shared_ptr<Socket> Client;
		size_t ContentLength = 0;
		String ParamString;
		//获取URL中带的值
		String GetParam(const String& key)const;
		//获取header里的值
		bool GetHeader(const String& key, String& value)const;
		////读取文件body里面的流 _Count每次读取大小 不会全部读完
		int ReadStream(String& buf, size_t _Count) const;
		//读取文件body里面的流 _Count每次读取大小 一下子读完
		size_t ReadStreamToEnd(String& body, size_t _Count)const;
	};
	struct Response
	{
		size_t  Status = 200;
		String Cookie;
		String Body;
		String Location;
		String ContentType;
		FileSystem::FileInfo* fileinfo = NULL;
		std::map<String, String> Headers;
		void SetContent(const String& body, const String& ContentType_ = "text/plain");
		void AddHeader(const String& key, const String& value);
		void RemoveHeader(const String& key);
		virtual ~Response();
	};

	typedef std::function<void(const Request&, Response&)> HttpHandler;
	class Server
	{
	private:
		ThreadPool* threadPool = NULL;
	public:
		//服务器上行速度
		size_t UpSize = 1024 * 1;//每次接收1KB
		//服务器下行速度
		size_t DownSize = 512 * 1024 * 1; //每次发送512KB
		String WebRoot;
		//绑定GET请求
		void Get(const String&, const HttpHandler&);
		//绑定POST请求
		void Post(const String&, const HttpHandler&);
		//开始监听HTTP协议请求
		bool Listen(const String& address, size_t port, int backlog = SOMAXCONN);
		Server();
		virtual ~Server();
	private:
		String Address;
		size_t Port;
		std::map<String, HttpHandler> PostFunc;
		std::map<String, HttpHandler> GetFunc;
		void ResponseHeader(Request& rq, Response& rp);
		//处理body部分
		void ResponseBody(Request& rq, Response& rp);
		//处理头部信息
		bool RegexValue(const String& content, const String& regex, String& result);
		bool ReceiveHeader(Request& rq);
		HttpHandler* HandleUrl(Request& rq, Response& rp);
		void Receive(const Socket client);
	};
}

