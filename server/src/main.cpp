#include "chess-server.hpp"
#include <boost/asio/io_context.hpp>
#include <iostream>

int main() {
    try {
        boost::asio::io_context io;
        ChessServer server(io, "127.0.0.1", 8080);
        server.start();
        std::cout << "Chess server listening on ws://127.0.0.1:8080" << std::endl;
        io.run();
    } catch (const std::exception& ex) {
        std::cerr << "Server error: " << ex.what() << std::endl;
        return 1;
    }
    return 0;
}
