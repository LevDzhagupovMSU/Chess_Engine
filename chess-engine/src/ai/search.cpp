#include "ai.hpp"
#include "board.hpp"
#include "piece.hpp"

#include <climits>
#include <utility>
#include <vector>

namespace ce {

namespace {
constexpr int MATE_SCORE = 1000000;
}

Move Ai::getAiMove(ChessBoard& board, Color ai_color, int depth) const {
    std::vector<std::vector<Move>> ai_moves;
    for(int k = 0; k < NUM_SQUARES; k++){
        if(board.pieces_[k] && board.pieces_[k]->getColor() == ai_color){
            auto moves = board.pieces_[k]->FINDPOSSIBLEMOVES(board);
            std::vector<Move> ai_legalMoves;
            for(const auto& move : moves){
                if(board.isLegalMove(move)) ai_legalMoves.push_back(move);
            }
            if(!ai_legalMoves.empty()) ai_moves.push_back(std::move(ai_legalMoves));
        }
    }
    if(ai_moves.empty()) return {-1, -1};
    auto evaluate = ai_EvaluateMoves(board, ai_moves, depth, ai_color);
    int bestScore = INT_MIN;
    Move bestMove = {-1, -1};
    for(size_t i = 0; i < ai_moves.size(); ++i) {
        for(size_t j = 0; j < ai_moves[i].size(); ++j) {
            if(evaluate[i][j] > bestScore) {
                bestScore = evaluate[i][j];
                bestMove = ai_moves[i][j];
            }
        }
    }
    return bestMove;
}

std::vector<std::vector<int>> Ai::ai_EvaluateMoves(
    ChessBoard& board, const std::vector<std::vector<Move>>& ai_moves, int depth, Color perspective) const {
    std::vector<std::vector<int>> evaluations;
    for(const auto& piece_ai_moves : ai_moves){
        std::vector<int> moves_evaluate;
        for(const auto& ai_move : piece_ai_moves){
            moves_evaluate.push_back(ai_EvaluateSingleMove(board, ai_move, depth, perspective, 1));
        }
        evaluations.push_back(moves_evaluate);
    }
    return evaluations;
}

int Ai::ai_EvaluateSingleMove(
    ChessBoard& board, Move ai_move, int depth, Color perspective, int ply) const {
    Color movingColor = board.pieces_[ai_move.from]->getColor();
    ChessBoard::ScopedMove applied(board, ai_move);
    Color opponentColor = (movingColor == white) ? black : white;
    std::vector<Move> opponentMoves;
    for(int i = 0; i < NUM_SQUARES; ++i) {
        if(board.pieces_[i] && board.pieces_[i]->getColor() == opponentColor) {
            auto moves = board.pieces_[i]->FINDPOSSIBLEMOVES(board);
            for(const auto& move : moves) {
                if(board.isLegalMove(move)) opponentMoves.push_back(move);
            }
        }
    }
    if(opponentMoves.empty()) {
        if(board.isCheck(opponentColor)) {
            return opponentColor == perspective ? -MATE_SCORE + ply : MATE_SCORE - ply;
        }
        return 0;
    }
    if(depth <= 1) {
        return evaluatePosition(board, perspective);
    }
    const bool maximizing = opponentColor == perspective;
    int bestScore = maximizing ? INT_MIN : INT_MAX;
    for(const auto& move : opponentMoves) {
        int score = ai_EvaluateSingleMove(board, move, depth - 1, perspective, ply + 1);
        if (maximizing) {
            if (score > bestScore) bestScore = score;
        } else {
            if (score < bestScore) bestScore = score;
        }
    }
    return bestScore;
}

} // namespace ce
