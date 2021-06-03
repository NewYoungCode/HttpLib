#include "HttpLinster.h"

namespace HttpLib {
	//Form的函数
	size_t Form::Read(char* buf, size_t count) {
		memset(buf, 0, count);
		for (size_t i = 0; i < DataCount; i++)
		{
			buf[i] = DataPos[i];
		}
		return 0;
	}
	//Request的函数
	String Request::GetParam(const String& key)const {
		std::map<String, String> Params;
		if (!ParamString.empty()) {
			std::vector<String> arr;
			ParamString.Split("&", arr);
			for (auto& it : arr) {
				size_t eq_ = it.find("=");
				String key_ = it.substr(0, eq_);
				String value = it.substr(eq_ + 1);
				if (key == key_) {
					return value;
				}
			}
		}
		return "";
	}
	bool  Request::GetHeader(const String& key, String& value) const {
		for (auto& it : Headers) {
			if (it.first == key) {
				value = it.second;
				return true;
			}
		}
		return false;
	}
	int  Request::ReadStream(String& buf, size_t _Count) const {
		buf.clear();
		Request* ptr = (Request*)this;
		if (ptr->ReadCount >= ContentLength || ptr->ContentLength == -1) {
			return 0;
		}
		if (!Temp.empty()) {
			buf.append(Temp.c_str(), Temp.size());
			ptr->ReadCount += Temp.size();
			ptr->Temp.clear();
			return buf.size();
		}

		std::shared_ptr<char> buf2(new char[_Count]);
		int len = Client.Receive(buf2.get(), _Count);
		if (len != -1 && len != 0) {
			buf.append(buf2.get(), len);
			ptr->ReadCount += len;
		}
		return len == -1 ? 0 : len;
	}
	size_t  Request::ReadStreamToEnd(String& body, size_t _Count)const {
		String* buf = new String;
		while (ReadStream(*buf, _Count) > 0)
		{
			body.append(buf->c_str(), buf->size());
		}
		return body.size();
	}
	//Response结构体的函数
	void Response::SetContent(const String& body, const String& ContentType_) {
		Body = body;
		ContentType = ContentType_;
	}
	void Response::AddHeader(const String& key, const String& value)
	{
		bool exist = false;
		for (auto& it : Headers) {
			if (it.first == key) {
				it.second = value;
				exist = true;
				break;
			}
		}
		if (!exist) {
			Headers.emplace(std::pair<String, String>(key, value));
		}
	}
	void Response::RemoveHeader(const String& key)
	{
		Headers.erase(key);
	}
	Response::~Response() {
		if (fileinfo) {
			delete  fileinfo;
		}
	}
}

namespace HttpLib {
	//HttpLinster的函数
	Server::Server()
	{
		WebRoot = Path::StartPath();
		threadPool = new ThreadPool(50);
	}
	Server::~Server()
	{
		if (threadPool) {
			delete threadPool;
		}
	}
	bool Server::RegexValue(const String& content, const String& regex, String& result)
	{
		std::regex regexExp(regex);
		std::smatch matchResult;
		if (std::regex_search(content, matchResult, regexExp)) {
			if (matchResult.size() >= 2)
			{
				result = (String)matchResult[1];
				return true;
			}
		}
		return false;
	}
	void Server::Get(const String& url, const HttpHandler& func)
	{
		GetFunc.emplace(std::pair<String, HttpHandler>(url, func));
	}
	void Server::Post(const String& url, const HttpHandler& func)
	{
		PostFunc.emplace(std::pair<String, HttpHandler>(url, func));
	}
	bool Server::Listen(const String& address, size_t port, int backlog)
	{
		this->Address = address;
		this->Port = port;
		Socket s;
		s.Bind(address, port);
		bool b = s.Listen(backlog);
		if (b) {
			printf("%s:%d started!\n", address.c_str(), port);
		}
		for (;;) {
			//单任务
			//Receive(s.Accep());
			//多任务
			threadPool->enqueue(&Server::Receive, this, s.Accep());
		}
		return b;
	}
	bool Server::ReceiveHeader(Request& rq) {
		bool ok = false;
		bool EndHead = false;

		std::shared_ptr<char> buf(new char[UpSize] {0});
		for (;;) {
			int len = rq.Client.Receive(buf.get(), UpSize);
			if (len == -1 || len == 0) {
				printf("%s连接断开!\n", rq.Client.Address.c_str());
				break;
			}
			if (rq.Header.empty()) {
				String tempStr = buf.get();
				size_t pos1 = tempStr.find("POST /");
				size_t pos2 = tempStr.find("GET /");
				//这里确定是否为http协议
				if (pos1 != 0 && pos2 != 0) {
					printf("no http !\n");
					break;
				}
				rq.Method = pos1 == 0 ? POST : GET;
			}
			if (!EndHead) {
				//最大支持报文2048字节
				if (rq.Header.size() > 2048) {
					printf("Maximum limit exceeded 2048 byte !\n");
					break;
				}
				rq.Header.append(buf.get(), len);
			}
			else {
				rq.Temp.append(buf.get(), len);
			}
			rq.HeadPos = rq.Header.find("\r\n\r\n");
			if (!EndHead && rq.HeadPos != String::npos) {
				//处理头部信息
				EndHead = true;
				rq.Temp = rq.Header.substr(rq.HeadPos + 4);
				rq.Header = rq.Header.substr(0, rq.HeadPos);
				size_t pos = rq.Header.find("/");
				for (size_t i = pos; i < rq.Header.size() - pos; i++)
				{
					if (rq.Header.at(i) == ' ') {
						break;
					}
					rq.RawUrl += rq.Header.at(i);
				}
				//取出url
				size_t what = rq.RawUrl.find("?");
				rq.Url = what != String::npos ? rq.RawUrl.substr(0, what) : rq.RawUrl;
				rq.ParamString = what != String::npos ? rq.RawUrl.substr(what + 1) : "";

				//插入header键值对
				std::vector<String> vstr;
				rq.Header.Split("\r\n", vstr);
				for (auto kv = vstr.begin() + 1; vstr.size() >= 2 && kv != vstr.end(); kv++)
				{
					size_t mhPos = (*kv).find(":");
					rq.Headers.emplace(std::pair<String, String>((*kv).substr(0, mhPos), (*kv).substr(mhPos + 2)));
				}

				//如果是POST方法取出content长度继续接收
				String contentLen;
				if (RegexValue(rq.Header.Replace( " ", ""), "Content-Length:(\\d+)", contentLen)) {
					try
					{
						rq.ContentLength = std::stoi(contentLen);
					}
					catch (const std::exception&)
					{
						rq.ContentLength = -1;
					}
				}

				//获取cookie
				rq.GetHeader("Cookie", rq.Cookie);
				//能执行到此处代表 报文头正常接受
				ok = true;
				break;
			}
		}
		return ok;
	}
	void Server::Receive(const Socket& client) {
		printf("%s:%d\n", client.Address.c_str(), client.Port);
		Request rq;
		Response rp;
		rq.Client = client;
		//接收请求头
		if (!ReceiveHeader(rq)) {
			printf("Termination of receiving, abnormal message\n");
			client.Close();
			return;
		}
		//打印出请求头部看看
		printf("%s\n", rq.Header.c_str());
		//处理请求头
		auto handler = HandleUrl(rq, rp);
		if (handler) {
			(*handler)(rq, rp);
		}
		//处理head响应
		ResponseHeader(rq, rp);
		//处理body响应
		ResponseBody(rq, rp);
		String keep_alive;
		if (rq.GetHeader("Connection", keep_alive) && keep_alive == "keep-alive") {
			printf("keep-alive\n");
			//继续接收数据
			threadPool->enqueue(&Server::Receive, this, rq.Client);
		}
		else {
			printf("Connection close\n");
			rq.Client.Close();
		}
	}
	HttpHandler* Server::HandleUrl(Request& rq, Response& rp) {
		//查找绑定的函数
		if (rq.Method == GET) {
			for (auto& it : GetFunc) {
				if (it.first == rq.Url) {
					return &it.second;
				}
			}
			//如果找不到绑定的函数 就看看是不是在请求文件
			String filename = WebRoot + "\\" + rq.Url;
			if (File::Exists(filename)) {
				String ext = String(Path::GetExtension(filename)).Replace(".", "");
				if (ext == "html" || ext == "htm") {
					rp.ContentType = "text/html";
				}
				else if (ext == "js") {
					rp.ContentType = "application/javascript";
				}
				else if (ext == "css") {
					rp.ContentType = "text/" + ext;
				}
				else if (ext == "jpg" || ext == "png" || ext == "bmp" || ext == "jpeg" || ext == "svg") {
					rp.ContentType = "image/" + ext;
					if (ext == "svg") {
						rp.ContentType.append("+xml");
					}
				}
				else  if (ext == "ico") {
					rp.ContentType = "image/x-icon";
				}

				else if (ext == "mp4") {
					rp.ContentType = "video/" + ext;
				}
				else {
					rp.ContentType = "application/octet-stream";
				}
				rp.Status = 200;
				rp.fileinfo = new FileSystem::FileInfo(filename);
			}
			else {
				rp.Status = 404;
			}
			return NULL;
		}
		else if (rq.Method == POST)
		{
			for (auto& it : PostFunc) {
				if (it.first == rq.Url) {
					return  &it.second;
				}
			}
		}
		return NULL;
	}
	void Server::ResponseHeader(Request& rq, Response& rp)
	{
		if (rp.fileinfo) {
			//获取文件最后修改的时间
			String lstChange = std::to_string(rp.fileinfo->__stat.st_mtime);
			String inm;
			if (rq.GetHeader("If-None-Match", inm) && inm.Trim() == lstChange) {
				//使用浏览器缓存
				rp.Status = 304;
			}
			else {
				//普通传输文件
				rp.Status = 200;
			}
			rp.AddHeader("ETag", lstChange);
		}
		//重定向响应
		if (!rp.Location.empty()) {
			rp.Status = 302;
			rp.AddHeader("Location", rp.Location);
		}
		//响应协议头
		String header("HTTP/1.1 " + std::to_string(rp.Status) + " OJBK \r\n");
		rp.AddHeader("Content-Length", std::to_string(rp.fileinfo ? rp.fileinfo->__stat.st_size : rp.Body.size()));
		rp.AddHeader("Accept-Ranges", "none");
		rp.AddHeader("Server", "LSkin Server 2.0");
		if (!rp.Cookie.empty()) {
			rp.AddHeader("Set-Cookie", rp.Cookie);//
		}
		if (!rp.ContentType.empty()) {
			rp.AddHeader("Content-Type", rp.ContentType);
		}
		//将用户的头部值加入
		for (auto& it : rp.Headers) {
			header.append(it.first + ": " + it.second + "\r\n");
		}
		header.append("\r\n");
		printf("---------------------------------------------------\n%s\n", header.c_str());//输出响应头部
		rq.Client.Write(header.c_str(), header.size());//发送响应头部
	}
	void Server::ResponseBody(Request& rq, Response& rp) {
		if (!rp.fileinfo) {
			if (rp.Body.size() != 0) {
				rq.Client.Write(rp.Body.c_str(), rp.Body.size());//普通响应body
			}
		}
		else {
			if (rp.Status != 304) { //如果不使用 缓存
				//普通响应文件

				std::shared_ptr<char> buf2(new char[DownSize] {0});
				size_t ct = 0;
				//读取文件
				while ((ct = rp.fileinfo->Read(buf2.get(), DownSize)) > 0)
				{
					//发送内存块
					if (ct != rq.Client.Write(buf2.get(), ct)) {
						printf("文件传输失败!\n");
						break;
					}
				}
			}
		}
	}
}