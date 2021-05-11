#include "HttpLinster.h"
using namespace HttpLib;
int main()
{  
	Server server;
	std::printf("base dir at [%s]\n started !", server.WebRoot.c_str());
	server.Get("/", [=](const Request& rq, Response& rp) {
		rp.Location = "/index.html";
		});
	server.Get("/test", [=](const  Request& rq, Response& rp) {
		rp.SetContent("{\"get\":\"LSkin Server 2.0!\"}", "application/json");
		});
	server.Post("/test", [=](const  Request& rq, Response& rp) {
		rp.SetContent("{\"post\":\"LSkin Server 2.0!\"}", "application/json");
		String body;
		rq.ReadStreamToEnd(body, 128);
		std::printf("%s\n", body.c_str());
		});
	server.Listen("0.0.0.0", 80);
	return 0;
}

