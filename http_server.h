#pragma once

#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <memory>
#include <functional>
#include <unordered_map>
#include <AK/WwiseAuthoringAPI/AkAutobahn/Client.h>


namespace beast = boost::beast;
namespace http = beast::http;
namespace net = boost::asio;
using tcp = net::ip::tcp;

class HttpServer;

using RequestHandler = std::function<void(const http::request<http::string_body>&, http::response<http::string_body>&, HttpServer&)>;

// HttpServer.
class HttpServer {
public:
  HttpServer(short port, AK::WwiseAuthoringAPI::Client& waapi_client);

  void Register(const std::string& path, RequestHandler handler);

  void Start();

  net::io_context io_context_;
  tcp::acceptor acceptor_;
  std::unordered_map<std::string, RequestHandler> routes_;
  AK::WwiseAuthoringAPI::Client waapi_client_;

  void accept();
};


// HttpSession.
class HttpSession : public std::enable_shared_from_this<HttpSession> {
public:
    HttpSession(tcp::socket socket, std::unordered_map<std::string, RequestHandler>& routes, HttpServer& server);

    void start();

    tcp::socket socket_;
    beast::flat_buffer buffer_;
    http::request<http::string_body> request_;
    http::response<http::string_body> response_;
    std::unordered_map<std::string, RequestHandler>& routes_;
    HttpServer& server_ref_;

    void readRequest();

    void handleRequest();

    void writeResponse();
};


bool ConfigureHttpRouter(HttpServer& server) noexcept;