#pragma once

namespace ce {

enum Color{white, black};

enum class PieceKind { pawn, knight, bishop, rook, queen, king };

constexpr int BOARD_SIZE = 8;
constexpr int NUM_SQUARES = 64;

// PST (piece-square tables) оценка позиции
extern const int PAWN_PST[64];
extern const int KNIGHT_PST[64];
extern const int BISHOP_PST[64];
extern const int ROOK_PST[64];
extern const int QUEEN_PST[64];
extern const int KING_PST[64];

} // namespace ce
