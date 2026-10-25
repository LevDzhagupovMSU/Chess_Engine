#include "game.hpp"

#include <stdexcept>
#include <utility>

namespace ce {

Game::Game() {
    updateStatus();
}

Game::Game(ChessBoard board, Color side_to_move)
    : board_(std::move(board)), side_to_move_(side_to_move) {
    if (board_.kingSquare(white) < 0 || board_.kingSquare(black) < 0) {
        throw std::invalid_argument("A game requires both kings");
    }
    restoreHalfmoveClock();
    updateStatus();
}

const ChessBoard& Game::board() const noexcept { return board_; }
Move Game::getAiMove(int depth) {
    if (status_ == GameStatus::checkmate || status_ == GameStatus::stalemate ||
        status_ == GameStatus::fifty_move_draw) {
        return {-1, -1};
    }
    return ai_.getAiMove(board_, side_to_move_, depth);
}
Color Game::sideToMove() const noexcept { return side_to_move_; }
GameStatus Game::status() const noexcept { return status_; }
std::size_t Game::halfmoveClock() const noexcept { return halfmove_clock_; }

bool Game::canClaimFiftyMoveDraw() const noexcept {
    return halfmove_clock_ >= 100 &&
           status_ != GameStatus::checkmate &&
           status_ != GameStatus::stalemate &&
           status_ != GameStatus::fifty_move_draw;
}

bool Game::claimFiftyMoveDraw() noexcept {
    if (!canClaimFiftyMoveDraw()) return false;
    status_ = GameStatus::fifty_move_draw;
    return true;
}

bool Game::canClaimFiftyMoveDraw(Move intended_move) {
    if (halfmove_clock_ != 99 ||
        status_ == GameStatus::checkmate ||
        status_ == GameStatus::stalemate ||
        status_ == GameStatus::fifty_move_draw ||
        intended_move.from < 0 || intended_move.from >= NUM_SQUARES) {
        return false;
    }
    const Piece* piece = board_.pieceAt(intended_move.from);
    return piece && piece->getColor() == side_to_move_ &&
           piece->getKind() != PieceKind::pawn &&
           board_.pieceAt(intended_move.to) == nullptr &&
           board_.isLegalMove(intended_move);
}

bool Game::claimFiftyMoveDraw(Move intended_move) {
    if (!canClaimFiftyMoveDraw(intended_move)) return false;
    status_ = GameStatus::fifty_move_draw;
    return true;
}

bool Game::makeMove(Move move) {
    if (status_ == GameStatus::checkmate || status_ == GameStatus::stalemate ||
        status_ == GameStatus::fifty_move_draw ||
        move.from < 0 || move.from >= NUM_SQUARES) {
        return false;
    }
    const Piece* piece = board_.pieceAt(move.from);
    if (!piece || piece->getColor() != side_to_move_) {
        return false;
    }
    if (!board_.makeMove(move)) {
        return false;
    }
    const MoveRecord& record = board_.getHistory().last();
    halfmove_clock_ = record.piece == PieceKind::pawn || record.captured
                          ? 0 : halfmove_clock_ + 1;
    side_to_move_ = side_to_move_ == white ? black : white;
    updateStatus();
    return true;
}

void Game::restoreHalfmoveClock() noexcept {
    const History& history = board_.getHistory();
    for (std::size_t i = history.size(); i > 0; --i) {
        const MoveRecord& record = history.at(i - 1);
        if (record.piece == PieceKind::pawn || record.captured) break;
        ++halfmove_clock_;
    }
}

void Game::updateStatus() {
    const bool check = board_.isCheck(side_to_move_);
    if (!board_.hasAnyLegalMove(side_to_move_)) {
        status_ = check ? GameStatus::checkmate : GameStatus::stalemate;
    } else {
        status_ = check ? GameStatus::check : GameStatus::ongoing;
    }
}

} // namespace ce
