/*******************************************************************************
The content of this file includes portions of the AUDIOKINETIC Wwise Technology
released in source code form as part of the SDK installer package.

Commercial License Usage

Licensees holding valid commercial licenses to the AUDIOKINETIC Wwise Technology
may use this file in accordance with the end user license agreement provided
with the software or, alternatively, in accordance with the terms contained in a
written agreement between you and Audiokinetic Inc.

  Copyright (c) 2023 Audiokinetic Inc.
*******************************************************************************/

#include <AK/WwiseAuthoringAPI/AkAutobahn/Client.h>
#include <AK/WwiseAuthoringAPI/waapi.h>
#include <iostream>
#include <cassert>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <iostream>
#include <memory>
#include <functional>
#include <unordered_map>
#include <rapidjson/document.h>


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

namespace beast = boost::beast;
namespace http = beast::http;
namespace net = boost::asio;
using tcp = net::ip::tcp;
using RequestHandler = std::function<void(const http::request<http::string_body>&, http::response<http::string_body>&)>;

class HttpSession : public std::enable_shared_from_this<HttpSession> {
public:
    HttpSession(tcp::socket socket, std::unordered_map<std::string, RequestHandler>& routes)
        : socket_(std::move(socket)), routes_(routes) {
    }

    void start() { read_request(); }

private:
    tcp::socket socket_;
    beast::flat_buffer buffer_;
    http::request<http::string_body> request_;
    http::response<http::string_body> response_;
    std::unordered_map<std::string, RequestHandler>& routes_;

    void read_request() {
        auto self = shared_from_this();
        http::async_read(socket_, buffer_, request_,
            [self](beast::error_code ec, std::size_t) {
                if (!ec) self->handle_request();
            });
    }

    void handle_request() {
        auto it = routes_.find(std::string(request_.target()));
        if (it != routes_.end()) {
            it->second(request_, response_);
        }
        else {
            response_.result(http::status::not_found);
            response_.set(http::field::content_type, "text/plain");
            response_.body() = "404 Not Found";
        }
        response_.prepare_payload();
        write_response();
    }

    void write_response() {
        auto self = shared_from_this();
        http::async_write(socket_, response_,
            [self](beast::error_code ec, std::size_t) {
                self->socket_.shutdown(tcp::socket::shutdown_send, ec);
            });
    }
};

class HttpServer {
public:
    HttpServer(net::io_context& ioc, short port)
        : acceptor_(ioc, tcp::endpoint(tcp::v4(), port)) {
        accept();
    }

    void add_route(const std::string& path, RequestHandler handler) {
        routes_[path] = std::move(handler);
    }

private:
    tcp::acceptor acceptor_;
    std::unordered_map<std::string, RequestHandler> routes_;

    void accept() {
        acceptor_.async_accept(
            [this](beast::error_code ec, tcp::socket socket) {
                if (!ec) std::make_shared<HttpSession>(std::move(socket), routes_)->start();
                accept();
            });
    }
};

bool configureHttpRouter(HttpServer& server) noexcept {
    try {
        server.add_route("/hello", [](const http::request<http::string_body>& req, http::response<http::string_body>& res) {
            std::cout << "request body: \n";
            rapidjson::Document doc;
            doc.Parse(req.body().c_str());
            if (doc.HasParseError()) {
                std::cout << "failed to parse json\n";
                res.result(http::status::not_found);
                return;
            }

            std::cout << doc["file"].GetString() << "\n";
            res.result(http::status::ok);
            res.body() = "Hello, World!";
            });

        server.add_route("/goodbye", [](const http::request<http::string_body>& req, http::response<http::string_body>& res) {
            res.result(http::status::ok);
            res.body() = "Goodbye!";
            });
    }
    catch (const std::exception& e) {
        std::cerr << "Server error: " << e.what() << "\n";
        return false;
    }
    return true;
}

int main(int argc, char* argv[]) {
	using namespace AK::WwiseAuthoringAPI;
    int waapi_port = 8080;
    int server_port = 5080;

	Client waapi_client;
	if (!waapi_client.Connect("127.0.0.1", waapi_port)) {
		std::cout << "failed to connect to wamp host\n";
		return -1;
	};
	std::cout << "successfully connected to host\n";
	
    // setup http server.
    net::io_context ioc;
    HttpServer server(ioc, server_port);
    if (!configureHttpRouter(server)) {
        std::cout << "failed to configure http router\n";
        return -1;
    }
    ioc.run();
    std::cout << "Server running on port " << server_port << "\n";
	return 0;
}