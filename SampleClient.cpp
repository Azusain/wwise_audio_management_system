#include <AK/WwiseAuthoringAPI/AkAutobahn/Client.h>
#include <iostream>
#include "http_server.h"
#include <AK/WwiseAuthoringAPI/waapi.h>

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