# HttpLib
c++ 实现http server 2.0

使用方法:
#include "HttpLinster.h"
using namespace HttpLib;
int main()
{
	Server server;
	std::printf("base dir at [%s]\n started !", server.WebRoot.c_str());
	server.WebRoot = "D:\\WebRoot";//设置前端目录

	server.Get("/", [=](const Request& rq, Response& rp) {
		//重定向
		rp.Location = "/index.html";
		});

 //绑定GET请求
	server.Get("/test", [=](const  Request& rq, Response& rp) {
		rp.SetContent("{\"get\":\"LSkin Server 2.0!\"}", "application/json");
		});

 //绑定POST请求接口
	server.Post("/test", [=](const  Request& rq, Response& rp) {
		rp.SetContent("{\"post\":\"LSkin Server 2.0!\"}", "application/json");
		String body;
		rq.ReadStreamToEnd(body, 128); //读取body所有内容
		std::printf("%s\n", body.c_str());
		});

//地址和端口
	server.Listen("0.0.0.0", 80);

	return 0;
}

