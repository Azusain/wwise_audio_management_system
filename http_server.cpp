#include <iostream>
#include <memory>
#include <functional>
#include <unordered_map>
#include <AK/WwiseAuthoringAPI/waapi.h>
#include "http_server.h"
#include <AK/WwiseAuthoringAPI/AkAutobahn/Client.h>
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
#include "json.hpp"
#include "ui.h"

using json = nlohmann::json;

// For official good pratices:
// https://www.audiokinetic.com/en/library/edge/?source=SDK&id=waapi_example_index.html.


// Implement HttpSession.
HttpSession::HttpSession(tcp::socket socket, std::unordered_map<std::string, RequestHandler>& routes, HttpServer& server)
  : socket_(std::move(socket)), routes_(routes), server_ref_(server) {
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
  // CORS.
  // Not only OPTIONS requests but GET requests need these headers.
  response_.set(http::field::access_control_allow_origin, "*");
  response_.set(http::field::access_control_allow_methods, "GET, POST, OPTIONS");
  response_.set(http::field::access_control_allow_headers, "Content-Type, Authorization");
  response_.result(http::status::ok);

  // call responding handler.
  if (request_.method() != http::verb::options) {
    auto it = routes_.find(std::string(request_.target()));
    if (it != routes_.end()) {
      it->second(request_, response_, server_ref_);
    }
    else {
      std::cerr << "invalid request from client\n";
      response_.result(http::status::not_found);
      response_.set(http::field::content_type, "text/plain");
      response_.body() = "404 Not Found";
    }
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
      if (!ec) std::make_shared<HttpSession>(std::move(socket), routes_, *this)->start();
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
    server.Register("/", [](const http::request<http::string_body>& req, http::response<http::string_body>& res, HttpServer& server) {
      res.result(http::status::ok);
      json resp_json{
        {"message", "Welcome to waapi control server"}
      };
      res.body() = resp_json.dump();
      });

    // Import audio files.
    // Ref: https://www.audiokinetic.com/zh/library/edge/?source=SDK&id=ak_wwise_core_audio_import.html
    server.Register("/import", [](const http::request<http::string_body>& req, http::response<http::string_body>& res, HttpServer& server) {
      // check and extract json fields.
      json incoming_request_json = json::parse(req.body());
      if (!incoming_request_json.contains("audioFiles")) {
        res.body() = json{ {"error", "missing json field: audioFiles"} }.dump();
        res.result(http::status::not_found);
        return;
      }
      // only valid while importing sou
      if (!incoming_request_json.contains("importLang")) {
        res.body() = json{ {"error", "missing json field: importLang"} }.dump();
        res.result(http::status::not_found);
        return;
      }
      json outgoing_request_json{
        {"importOperation", "useExisting"},
        {"imports", incoming_request_json["audioFiles"]}
      };
      auto& import_lang = incoming_request_json["importLang"].get<std::string>();
      if (import_lang != "") {
        outgoing_request_json["default"] = json{
          {"importLanguage", import_lang}
        };
      }
      std::string result_str;
      int ret = server.waapi_client_.Call(ak::wwise::core::audio::import, outgoing_request_json.dump().c_str(), "{}", result_str);
      if (!ret) {
        std::cerr << "failed to import audio files: " << result_str << "\n";
        res.body() = result_str;
        res.result(http::status::not_found);
        return;
      }
      res.body() = json{ {"message", "success"} }.dump();
      res.result(http::status::ok);
      });

    // Get children.
    server.Register("/children", [](const http::request<http::string_body>& req, http::response<http::string_body>& res, HttpServer& server) {
      json ui_req_json = json::parse(req.body());
      auto parent_path = ui_req_json["parent"].get<std::string>();
      json req_json{
        {"waql", "$ \"" + parent_path + "\"" + "select children"},
      };
      json opt_json{
        {"return", {"path", "id"}}
      };
      std::string result_str;
      int ret = server.waapi_client_.Call(ak::wwise::core::object::get, req_json.dump().c_str(), opt_json.dump().c_str(), result_str);
      json ret_json = json::parse(result_str);
      if (!ret) {
        res.body() = result_str;
        res.result(http::status::not_found);
        return;
      }
      res.body() = result_str;
      res.result(http::status::ok);
      });

    // Select Files.
    server.Register("/select", [](const http::request<http::string_body>& req, http::response<http::string_body>& res, HttpServer& server) {
      std::vector<std::string> selected_files;
      if (!OpenFileDialogMultiSelect(selected_files)) {
        json resp{
          {"error", "error occurs while selecting files"}
        };
        res.body() = resp.dump();
        res.result(http::status::not_found);
        return;
      };
      json selected_files_json = json::array();
      for (auto file : selected_files) {
        selected_files_json.push_back(file);
      }
      json resp{
        {"files", selected_files_json}
      };
      res.body() = resp.dump();
      res.result(http::status::ok);
      return;
      });

    server.Register("/languages", [](const http::request<http::string_body>& req, http::response<http::string_body>& res, HttpServer& server) {
      auto& client = server.waapi_client_;
      json req_json{
        {"from",{{"ofType", {"Language"}}}}
      };
      json opt_json{
        {"return", {"name"}}
      };
      std::string result_str;
      int ret = client.Call(ak::wwise::core::object::get, req_json.dump().c_str(), opt_json.dump().c_str(), result_str);
      if (!ret) {
        std::cerr << "failed to sync languages\n";
        res.body() = result_str;
        res.result(http::status::not_found);
        return;
      }
      res.body() = result_str;
      res.result(http::status::ok);
      });

    server.Register("/commands", [](const http::request<http::string_body>& req, http::response<http::string_body>& res, HttpServer& server) {
      auto& client = server.waapi_client_;
      std::string result_str;
      int ret = client.Call(ak::wwise::ui::commands::getCommands, "{}", "{}", result_str);
      res.body() = result_str;
      if (!ret) {
        std::cerr << "failed to list commands\n";
        res.result(http::status::not_found);
        return;
      }
      res.result(http::status::ok);
      return;
    });

    server.Register("/findwwise", [](const http::request<http::string_body>& req, http::response<http::string_body>& res, HttpServer& server) {
      auto& client = server.waapi_client_;
      std::string result_str;
      int ret = client.Call(ak::wwise::ui::bringToForeground, "{}", "{}", result_str);
      if (!ret) {
        std::cerr << "failed to bring Wwise to foreground: " << result_str << "\n";
        res.result(http::status::not_found);
        return;
      }
      res.result(http::status::ok);
      return;
    });

    server.Register("/getSelectedPath", [](const http::request<http::string_body>& req, http::response<http::string_body>& res, HttpServer& server) {
      auto& client = server.waapi_client_;
      json opt_json{
        {"return", {"path"}}
      };
      std::string result_str;
      int ret = client.Call(ak::wwise::ui::getSelectedObjects, "{}", opt_json.dump().c_str(), result_str);
      res.body() = result_str;
      if (!ret) {
        std::cerr << "failed to get selected path: " << result_str << "\n";
        res.result(http::status::not_found);
        return;
      }
      res.result(http::status::ok);
      return;
    });

    server.Register("/highlight", [](const http::request<http::string_body>& req, http::response<http::string_body>& res, HttpServer& server) {
      auto& client = server.waapi_client_;
      json req_json = json::parse(req.body());
      if (!req_json.contains("id")) {
        res.body() = json{ {"error", "missing json field: id"} }.dump();
        return;
      }
      json outgoing_req_json{
        {"command", "FindInProjectExplorerSyncGroup1"},
        {"objects", {req_json["id"].get<std::string>()}}
      };
      std::string result_str;
      int ret = client.Call(ak::wwise::ui::commands::execute, outgoing_req_json.dump().c_str(), "{}", result_str);
      if (!ret) {
        std::cerr << "failed to highlight object in Wwise\n";
        res.result(http::status::not_found);
        res.body() = result_str;
        return;
      }
      res.result(http::status::ok);
      return;
      });

    server.Register("/guid", [](const http::request<http::string_body>& req, http::response<http::string_body>& res, HttpServer& server) {
      auto& client = server.waapi_client_;
      // TODO: json::parse() may throw exception...
      json req_json = json::parse(req.body());
      if (!req_json.contains("path")) {
        res.body() = json{ {"error", "missing json field: path"} }.dump();
        return;
      }
      std::string path = req_json["path"].get<std::string>();
      json outgoing_req_json{
        {"from", {{"path", {path}}}},
      };
      json opt_json{
        {"return", {"id"}}
      };
      std::string result_str;
      int ret = client.Call(ak::wwise::core::object::get, outgoing_req_json.dump().c_str(), opt_json.dump().c_str(), result_str);
      res.body() = result_str;
      if (!ret) {
        std::cerr << "failed to get GUID from path: " << path << "\n";
        res.result(http::status::not_found);
        return;
      }
      res.body() = result_str;
      res.result(http::status::ok);
      return;
      });
  }

  catch (const std::exception& e) {
    std::cerr << "Server error: " << e.what() << "\n";
    return false;
  }
  return true;
}