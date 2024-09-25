#ifndef bot_h
#define bot_h

#include "boost_1_86_0/boost/asio.hpp"
#include "boost_1_86_0/boost/asio/ssl.hpp"
#include "boost_1_86_0/boost/beast.hpp"
#include "json.hpp"

#include <chrono>
#include <thread>

#include <iostream>
#include <vector>

#include <cctype>

using namespace boost::beast;
using namespace boost::asio;

class tgbot{
public:
    std::vector<std::function<void(nlohmann::json update)>> _handlers;

    void set_functions(std::function<void(nlohmann::json update)> handler){
        _handlers.push_back(handler);
    }
    void update_schedule(){
        system(R"("/home/almas/Рабочий стол/telegramapicpp/utils/work/bin/python" "/home/almas/Рабочий стол/telegramapicpp/utils/parser.py")");
    }
    http::response<http::string_body> _send_request(std::string url, std::string path){
        io_service svc;

        ssl::context ctx(ssl::context::sslv23_client);
        ssl::stream<ip::tcp::socket> ssocket = { svc, ctx };
        ip::tcp::resolver resolver(svc);
        auto it = resolver.resolve(url, "443");
        connect(ssocket.lowest_layer(), it);
        ssocket.handshake(ssl::stream_base::handshake_type::client);
        http::request<http::string_body> req{ http::verb::get, path, 11 };
        req.set(http::field::host, url);
        http::write(ssocket, req);
        http::response<http::string_body> res;
        flat_buffer buffer;
        http::read(ssocket, buffer, res);
        return res;
    }
    
    std::string _token = "7263851876:AAEM2YAISTD4Xhx750aniKJZfIOJ3Cg5HdQ";
    std::string _url = "api.telegram.org";

    int64_t last_update_id{};
    int64_t update_id{};

    nlohmann::json edit_message(int64_t chat_id, std::string text, int64_t message_id, std::string reply_markup = "", std::string parse_mode = "HTML"){
        return send_request("/editMessageText?chat_id=" + std::to_string(chat_id) + "&text=" + text + "&message_id=" + std::to_string(message_id) + "&reply_markup=" + reply_markup + "&parse_mode=" + parse_mode);
    }
    
    nlohmann::json send_message(int64_t chat_id, std::string text, std::string reply_markup = ""){
        return send_request("/sendMessage?chat_id=" + std::to_string(chat_id) + "&text=" + text + "&reply_markup=" + reply_markup);
    }

    void handle_update(nlohmann::json update){
        if (!update["message"]["text"].is_null()){
            _handlers[0](update); // handle text
        }
        else if (!update["callback_query"].is_null()){
            _handlers[1](update); // handle callback's
        } 
        else{
            std::cout  << "Неизвестно " << update << "\n";
        }

    }
    nlohmann::json send_request(std::string method){ // method = /getMe
        while (method.find('\n') != -1){
            method.replace(method.find('\n'), 1, "%0A");
        }
        nlohmann::json result = nlohmann::json::parse(_send_request(_url,  "/bot" + _token + method).body());
        if (result["ok"] == true){
            return result;
        }
        std::cout << result << "\n";
        std::cout << "error result = false\n";
        throw;
    }
    void start(){
        while (true){
            // get updates
            try{
                nlohmann::json updates = send_request("/getupdates?offset=" + std::to_string(last_update_id + 1));
                for (int index = 0; index < updates["result"].size(); index++){
                    last_update_id = updates["result"][index]["update_id"];
                    handle_update(updates["result"][index]);
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(70));
            }
            catch(const std::exception &e){
                std::cout << "ERROR\n";
                std::cout << e.what() << '\n';
            }
            
        }
    }
};

tgbot bot;

#endif // bot