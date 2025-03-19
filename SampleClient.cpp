#include <AK/WwiseAuthoringAPI/AkAutobahn/Client.h>
#include <iostream>
#include "http_server.h"
#include <AK/WwiseAuthoringAPI/waapi.h>

//int main(int argc, char** argv)
//{
//	std::string testToRun;
//
//	if (argc < 2)
//	{
//		testToRun = "HelloWorld";
//	}
//	else
//	{
//		testToRun = argv[1];
//	}
//	
//	if (testToRun == "HelloWorld")
//	{	
//		HelloWorld();
//	}
//	else if (testToRun == "TestMonitoring")
//	{
//		TestMonitoring();
//	}
//	else if (testToRun == "TestWampClient")
//	{
//		TestWampClient();
//	}
//	else if (testToRun == "TestWampClientJsonString")
//	{
//		TestWampClientJsonString();
//	}
//	else if (testToRun == "TestCustomCommands")
//	{
//		TestCustomCommands();
//	}
//	else if (testToRun == "TestErrors")
//	{
//		TestErrors();
//	}
//	else if (testToRun == "PerfTest")
//	{
//		PerfTest();
//	}
//	else if (testToRun == "StressTest")
//	{
//		StressTest(argc, argv);
//	}
//	else if (testToRun == "ScreenCapture")
//	{
//		ScreenCapture();
//	}
//	else if (testToRun == "TestAll")
//	{
//		HelloWorld();
//		TestWampClient();
//		TestWampClientJsonString();
//		TestCustomCommands();
//		PerfTest();
//		TestErrors();
//		ScreenCapture();
//	}
//	else
//	{
//		std::cout << "Usage: SampleClient [HelloWorld|TestWampClient|TestWampClientJsonString|TestMonitoring|TestCustomCommands|TestErrors|PerfTest|ScreenCapture|TestAll]" << std::endl;
//		return 1;
//	}
//
//	return 0;
//}


int main(int argc, char* argv[]) {
	using namespace AK::WwiseAuthoringAPI;
    int waapi_port = 8080;
    int server_port = 5080;

	// TODO: this may lead to some concurrency problems.
	Client waapi_client;
	if (!waapi_client.Connect("127.0.0.1", waapi_port)) {
		std::cout << "failed to connect to wamp host\n";
		return -1;
	};
	std::cout << "successfully connected to host\n";




    // setup http server.
    HttpServer server(server_port, waapi_client);
    if (!ConfigureHttpRouter(server)) {
        std::cout << "failed to configure http router\n";
        return -1;
    }
	std::cout << "Server running on port " << server_port << "\n";
	server.Start();

	return 0;
}