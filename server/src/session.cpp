#include "session.hpp"
#include "serializer.hpp"

#include <boost/beast/core/buffers_to_string.hpp>
#include <boost/beast/core/role.hpp>
#include <boost/beast/websocket/stream_base.hpp>
#include <boost/system/detail/error_code.hpp>
#include <cstdint>
#include <memory>
#include <string>
#include <utility>

Session::Session(boost::asio::ip::tcp::socket socket, int id, std::string client_ip,
                 uint16_t port, stop_callback on_stop) :
    web_socket_(std::move(socket)), session_id(id), client_ip_(std::move(client_ip)),
    client_port_(port), stop_callback_(std::move(on_stop)) {}

void Session::start(){
    read_server();
}

void Session::stop(){
    if (finished_) {
        return;
    }
    finished_ = true;

    boost::system::error_code ec;
    if (web_socket_.is_open()) {
        web_socket_.close({boost::beast::websocket::close_code::normal,
                           "Shutting down"}, ec);
    }
    if (!web_socket_.is_open() || ec) {
        boost::system::error_code ignored;
        web_socket_.next_layer().socket().close(ignored);
    }
    stop_callback_(shared_from_this());
}

void Session::read_server(){
    web_socket_.set_option(boost::beast::websocket::stream_base::timeout::suggested(boost::beast::role_type::server));   

    auto self = shared_from_this();
    web_socket_.async_accept([self](const boost::system::error_code& ec){
        if(!ec){
            self->read_message();
        }
        else{
            self->error_handle(error_type::lost_connection);
        }
    });
}

void Session::read_message() {
    auto self = shared_from_this();
    web_socket_.async_read(buffer_, [this, self](const boost::system::error_code& ec, std::size_t){
        if (finished_) {
            return;
        }

        if(!ec){
            std::string message = boost::beast::buffers_to_string(self->buffer_.data());
            self->buffer_.consume(self->buffer_.size());
            const auto player_move = get_player_move(message);

            if(!player_move || !game_.makeMove(*player_move)){
                self->error_handle(error_type::bad_move);
                return;
            }

            if (finish_game()) return;

            const ce::Move ai_move = game_.getAiMove(4);
            if (!game_.makeMove(ai_move)) {
                self->error_handle(error_type::lost_connection);
                return;
            }
            self->send_ai_move(ai_move);

        } else {
            self->error_handle(error_type::lost_connection);
        }
    });
}

void Session::send_ai_move(ce::Move ai_move){
    if (finished_) {
        return;
    }

    auto j_move = std::make_shared<std::string>(Serializer::aiMoveToJson(ai_move).dump());

    auto self = shared_from_this();
    web_socket_.async_write(boost::asio::buffer(*j_move), [self, j_move](const boost::system::error_code& ec, std::size_t) {
        if(ec) {
            self->error_handle(error_type::lost_connection);
        } else {
            if (!self->finish_game()) {
                self->read_message();
            }
        }
    });
}

void Session::send_bad_move(){
    if (finished_) {
        return;
    }

    auto j_bd = std::make_shared<std::string>(Serializer::badMoveToJson().dump());

    auto self = shared_from_this();
    web_socket_.async_write(boost::asio::buffer(*j_bd), [self, j_bd](const boost::system::error_code& ec, std::size_t){
        if(ec){
            self->error_handle(error_type::lost_connection);
        } else {
            self->read_message();
        }
    });
}

void Session::send_result(const std::string& result) {
    if (finished_ || result_pending_) {
        return;
    }
    result_pending_ = true;
    nlohmann::json j;
    j["type"] = "result";
    j["result"] = result;

    auto msg = std::make_shared<std::string>(j.dump());
    auto self = shared_from_this();
    web_socket_.async_write(boost::asio::buffer(*msg),
        [self, msg](const boost::system::error_code& ec, std::size_t) {
            if (ec) {
                self->error_handle(error_type::lost_connection);
            } else {
                self->stop();
            }
        });
}

std::optional<ce::Move> Session::get_player_move(const std::string& message) const{
    return Serializer::playerMoveFromJson(message);
}

bool Session::finish_game() {
    if (game_.canClaimFiftyMoveDraw()) game_.claimFiftyMoveDraw();
    if (game_.status() == ce::GameStatus::fifty_move_draw) {
        end_handle(game_end::draw);
    } else if (game_.status() == ce::GameStatus::stalemate) {
        end_handle(game_end::stalemate);
    } else if (game_.status() == ce::GameStatus::checkmate) {
        end_handle(game_.sideToMove() == ce::black ? game_end::win : game_end::lost);
    } else {
        return false;
    }
    return true;
}
