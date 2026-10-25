#pragma once

#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <cstdint>
#include <iostream>
#include <memory>
#include <vector>
#include <boost/asio.hpp>

#include "session.hpp"

class ChessServer {
    boost::asio::io_context& io_;
    boost::asio::ip::tcp::acceptor acceptor_;
    
    std::vector<std::shared_ptr<Session>> sessions_;
    std::string address_;

    uint16_t port_;

    void start_accept();

    void create_session(std::string client_ip, uint16_t port, boost::asio::ip::tcp::socket socket);
    void delete_session(std::shared_ptr<Session> session);

public:
    ChessServer(boost::asio::io_context& io, const std::string& address, uint16_t port);

    ~ChessServer();

    ChessServer(const ChessServer&) = delete;
    ChessServer& operator=(const ChessServer&) = delete;
    ChessServer(ChessServer&&) = delete;
    ChessServer& operator=(ChessServer&&) = delete;

    void start();
    void stop();
};