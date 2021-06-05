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
	//���Կ���̨���
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
	//����ṹ��
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
		//��ȡURL�д���ֵ
		String GetParam(const String& key)const;
		//��ȡheader���ֵ
		bool GetHeader(const String& key, String& value)const;
		////��ȡ�ļ�body������� _Countÿ�ζ�ȡ��С ����ȫ������
		int ReadStream(String& buf, size_t _Count) const;
		//��ȡ�ļ�body������� _Countÿ�ζ�ȡ��С һ���Ӷ���
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
		//�����������ٶ�
		size_t UpSize = 1024 * 1;//ÿ�ν���1KB
		//�����������ٶ�
		size_t DownSize = 512 * 1024 * 1; //ÿ�η���512KB
		String WebRoot;
		//��GET����
		void Get(const String&, const HttpHandler&);
		//��POST����
		void Post(const String&, const HttpHandler&);
		//��ʼ����HTTPЭ������
		bool Listen(const String& address, size_t port, int backlog = SOMAXCONN);
		Server();
		virtual ~Server();
	private:
		String Address;
		size_t Port;
		std::map<String, HttpHandler> PostFunc;
		std::map<String, HttpHandler> GetFunc;
		void ResponseHeader(Request& rq, Response& rp);
		//����body����
		void ResponseBody(Request& rq, Response& rp);
		//����ͷ����Ϣ
		bool RegexValue(const String& content, const String& regex, String& result);
		bool ReceiveHeader(Request& rq);
		HttpHandler* HandleUrl(Request& rq, Response& rp);
		void Receive(const Socket client);
	};
}

