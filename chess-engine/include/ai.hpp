#pragma once

#include "move.hpp"

#include <vector>

namespace ce {

class ChessBoard;

class Ai {
public:
    Move getAiMove(ChessBoard& board, Color ai_color, int depth) const;
    int evaluatePosition(const ChessBoard& board, Color perspective) const;

private:
    std::vector<std::vector<int>> ai_EvaluateMoves(
        ChessBoard& board, const std::vector<std::vector<Move>>& ai_moves,
        int depth, Color perspective) const;
    int ai_EvaluateSingleMove(
        ChessBoard& board, Move ai_move, int depth, Color perspective, int ply) const;
};

} // namespace ce
