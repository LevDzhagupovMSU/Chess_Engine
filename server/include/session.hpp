#pragma once

#include "game.hpp"

#include <boost/asio/ip/tcp.hpp>
#include <boost/beast/core/tcp_stream.hpp>
#include <boost/beast/websocket/stream.hpp>
#include <cstdint>
#include <functional>
#include <iostream>
#include <memory>
#include <optional>
#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <string>

enum class error_type {lost_connection, bad_move};
enum class game_end {win, lost, draw, stalemate}; // относительно игрока


class Session : public std::enable_shared_from_this<Session> {
public:
    using stop_callback = std::function<void(std::shared_ptr<Session>)>;

private:
    boost::beast::websocket::stream<boost::beast::tcp_stream> web_socket_;
    boost::asio::streambuf buffer_;

    ce::Game game_;

    int session_id;
    std::string client_ip_;
    uint16_t client_port_;

    stop_callback stop_callback_;

    bool finished_{false};
    bool result_pending_{false};

    void read_server();
    void read_message();

    void error_handle(error_type error);
    void end_handle(game_end end_type);

    void send_bad_move();
    void send_ai_move(ce::Move ai_move);
    void send_result(const std::string& result);

    std::optional<ce::Move> get_player_move(const std::string& message) const;

    bool finish_game();

public:
    Session(boost::asio::ip::tcp::socket socket, int id, std::string 
        client_ip, uint16_t port, stop_callback on_stop);

    void start();
    
    void stop();
};
