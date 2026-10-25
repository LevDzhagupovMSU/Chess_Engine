#include "chess-server.hpp"
#include "session.hpp"
#include <algorithm>
#include <boost/asio/ip/address.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/system/detail/error_code.hpp>
#include <cstdint>

ChessServer::ChessServer(boost::asio::io_context& io, const std::string& address, uint16_t port) :
    io_(io), acceptor_(io, boost::asio::ip::tcp::endpoint(
        boost::asio::ip::make_address(address), port)), address_(address), port_(port) {};
    
ChessServer::~ChessServer(){
    stop();
}

void ChessServer::start(){
    start_accept();
}

void ChessServer::start_accept(){
    if (!acceptor_.is_open()) return;
    acceptor_.async_accept([this](const boost::system::error_code& ec, boost::asio::ip::tcp::socket socket){
        if(ec){
            if (acceptor_.is_open()) {
                std::cerr << ec.what() << std::endl;
                start_accept();
            }
            return;
        }

        std::string client_ip = socket.remote_endpoint().address().to_string();
        uint16_t client_port = socket.remote_endpoint().port();

        create_session(client_ip, client_port, std::move(socket));
        start_accept();
    });
}

void ChessServer::create_session(std::string client_ip, uint16_t port, boost::asio::ip::tcp::socket socket){
    auto session = std::make_shared<Session>(std::move(socket), sessions_.size(), client_ip, port,
    [this](std::shared_ptr<Session> stopped_session){
        delete_session(stopped_session);
    }); 
    session->start();
    sessions_.push_back(session);
}

void ChessServer::delete_session(std::shared_ptr<Session> session){
    sessions_.erase(std::remove(sessions_.begin(), sessions_.end(), session),sessions_.end());
}

void ChessServer::stop(){
    boost::system::error_code ec;
    acceptor_.close(ec);
    while (!sessions_.empty()) {
        sessions_.front()->stop();
    }
}
