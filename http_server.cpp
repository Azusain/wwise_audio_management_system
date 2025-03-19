#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <iostream>
#include <memory>
#include <functional>
#include <unordered_map>
#include "http_server.h"

// Implement HttpSession.
HttpSession::HttpSession(tcp::socket socket, std::unordered_map<std::string, RequestHandler>& routes, AK::WwiseAuthoringAPI::Client& waapi_client)
    : socket_(std::move(socket)), routes_(routes), waapi_client_(waapi_client) {}

void HttpSession::start() { readRequest(); }

void HttpSession::readRequest() {
    auto self = shared_from_this();
    http::async_read(socket_, buffer_, request_,
        [self](beast::error_code ec, std::size_t) {
            if (!ec) self->handleRequest();
        });
}

void HttpSession::handleRequest() {
    auto it = routes_.find(std::string(request_.target()));
    if (it != routes_.end()) {
        it->second(request_, response_, waapi_client_);
    }
    else {
        response_.result(http::status::not_found);
        response_.set(http::field::content_type, "text/plain");
        response_.body() = "404 Not Found";
    }
    response_.prepare_payload();
    writeResponse();
}

void HttpSession::writeResponse() {
    auto self = shared_from_this();
    http::async_write(socket_, response_,
        [self](beast::error_code ec, std::size_t) {
            self->socket_.shutdown(tcp::socket::shutdown_send, ec);
        });
}

// Implement HttpServer.
HttpServer::HttpServer(short port, AK::WwiseAuthoringAPI::Client& waapi_client)
   : acceptor_(io_context_, tcp::endpoint(tcp::v4(), port)) {
    accept();
}

void HttpServer::Register(const std::string& path, RequestHandler handler) {
    routes_[path] = std::move(handler);
}

void HttpServer::accept() {
    acceptor_.async_accept(
        [this](beast::error_code ec, tcp::socket socket) {
            if (!ec) std::make_shared<HttpSession>(std::move(socket), routes_, waapi_client_)->start();
            accept();
        });
}

void HttpServer::Start() {
    io_context_.run();
}

// Register router.
bool ConfigureHttpRouter(HttpServer& server) noexcept {
    try {
        server.Register("/", [](const http::request<http::string_body>& req, http::response<http::string_body>& res, AK::WwiseAuthoringAPI::Client& client) {
            //std::cout << "request body: \n";
            //rapidjson::Document doc;
            //doc.Parse(req.body().c_str());
            //if (doc.HasParseError()) {
            //    std::cout << "failed to parse json\n";
            //    res.result(http::status::not_found);
            //    return;
            //}
            //std::cout << doc["file"].GetString() << "\n";
            res.result(http::status::ok);
            res.body() = "Welcome to waapi control server";
        });

        // Import audio files.
        server.Register("/import", [](const http::request<http::string_body>& req, http::response<http::string_body>& res, AK::WwiseAuthoringAPI::Client& client) {
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