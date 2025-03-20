#include <iostream>
#include <memory>
#include <functional>
#include <unordered_map>
#include <AK/WwiseAuthoringAPI/waapi.h>
#include "http_server.h"
#include <AK/WwiseAuthoringAPI/AkAutobahn/AkJson.h>
#include <AK/WwiseAuthoringAPI/AkAutobahn/AkVariant.h>
#include <AK/WwiseAuthoringAPI/AkAutobahn/Client.h>
#include <AK/WwiseAuthoringAPI/AkJsonBase.h>
#include <exception>
#include <string>
#include <type_traits>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/socket_base.hpp>
#include <boost/beast/core/error.hpp>
#include <boost/beast/http/field.hpp>
#include <boost/beast/http/impl/read.hpp>
#include <boost/beast/http/impl/write.hpp>
#include <boost/beast/http/message_fwd.hpp>
#include <boost/beast/http/status.hpp>
#include <boost/beast/http/string_body_fwd.hpp>
#include "RapidJsonUtils.h"
#include "json.hpp"

// For official good pratices:
// https://www.audiokinetic.com/en/library/edge/?source=SDK&id=waapi_example_index.html.


// Implement HttpSession.
HttpSession::HttpSession(tcp::socket socket, std::unordered_map<std::string, RequestHandler>& routes, AK::WwiseAuthoringAPI::Client& waapi_client)
  : socket_(std::move(socket)), routes_(routes), waapi_client_(waapi_client) {
}

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
  : acceptor_(io_context_, tcp::endpoint(tcp::v4(), port)), waapi_client_(waapi_client) {
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
  using namespace AK::WwiseAuthoringAPI;
  using json = nlohmann::json;
  
  try {
    // Default response.
    server.Register("/", [](const http::request<http::string_body>& req, http::response<http::string_body>& res, AK::WwiseAuthoringAPI::Client& waapi_client) {
      res.result(http::status::ok);
      json resp_json{
        {"message", "Welcome to waapi control server"}
      };
      res.body() = resp_json.dump();
      });

    // Import audio files.
    // Ref: https://www.audiokinetic.com/zh/library/edge/?source=SDK&id=ak_wwise_core_audio_import.html
    server.Register("/import", [](const http::request<http::string_body>& req, http::response<http::string_body>& res, AK::WwiseAuthoringAPI::Client& waapi_client) {
      AkJson::Array imported_objects;
      // parse request.
      rapidjson::Document doc;
      doc.Parse(req.body().c_str());
      if (doc.HasParseError()) {
        res.body() = "invalid json format";
        return;
      }
      if (doc.HasMember("audioFiles")) {
        auto& audioFiles = doc["audioFiles"];
        if (!audioFiles.IsArray()) {
          std::cerr << "audioFiles should be an array\n";
          return;
        }
        for (auto it = audioFiles.Begin(); it != audioFiles.End(); ++it) {
          imported_objects.emplace_back(AkJson::Map{
              {"audioFile", AkVariant((*it)["audioFile"].GetString())},
              {"objectPath", AkVariant((*it)["objectPath"].GetString())}
            });
        }
      }
      else {
        std::cerr << "invalid request arguments\n";
        return;
      }
      // response.
      AkJson request_json(AkJson::Map{
          {"importOperation", AkVariant("useExisting")},
          {"imports", imported_objects}
        });
      AkJson result_json;

      std::string result_str;
      int ret = waapi_client.Call(ak::wwise::core::audio::import, request_json, AkJson(AkJson::Type::Map), result_json);
      if (!ret) {
        std::cout << "failed to import audio files\n";
        res.result(http::status::not_found);
        return;
      }
      res.result(http::status::ok);
      });
  }
  catch (const std::exception& e) {
    std::cerr << "Server error: " << e.what() << "\n";
    return false;
  }
  return true;
}