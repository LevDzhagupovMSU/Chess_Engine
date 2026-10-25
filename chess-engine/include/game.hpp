#pragma once

#include "board.hpp"
#include "ai.hpp"

#include <cstddef>

namespace ce {

enum class GameStatus { ongoing, check, checkmate, stalemate, fifty_move_draw };

class Game {
    ChessBoard board_;
    Ai ai_;
    Color side_to_move_ = white;
    GameStatus status_ = GameStatus::ongoing;
    std::size_t halfmove_clock_ = 0;

    void updateStatus();
    void restoreHalfmoveClock() noexcept;
public:
    Game();
    explicit Game(ChessBoard board, Color side_to_move = white);

    const ChessBoard& board() const noexcept;
    Color sideToMove() const noexcept;
    GameStatus status() const noexcept;
    std::size_t halfmoveClock() const noexcept;
    bool canClaimFiftyMoveDraw() const noexcept;
    bool claimFiftyMoveDraw() noexcept;
    bool canClaimFiftyMoveDraw(Move intended_move);
    bool claimFiftyMoveDraw(Move intended_move);
    bool makeMove(Move move);
    Move getAiMove(int depth);
};

} // namespace ce
