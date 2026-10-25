#include "ai.hpp"
#include "board.hpp"
#include "piece.hpp"
#include "types.hpp"

#include <cstdlib>

namespace ce {

namespace {

int positionalBonus(PieceKind kind, int index) {
    // Legacy minor- and major-piece tables include a material baseline. Pawn values
    // are scaled so that square preference does not exceed a pawn's value.
    switch (kind) {
    case PieceKind::pawn: return PAWN_PST[index] / 4;
    case PieceKind::knight: return KNIGHT_PST[index] - 320;
    case PieceKind::bishop: return BISHOP_PST[index] - 330;
    case PieceKind::rook: return ROOK_PST[index] - 500;
    case PieceKind::queen: return QUEEN_PST[index] - 1000;
    case PieceKind::king: return KING_PST[index];
    }
    return 0;
}

bool attacksSquare(const Piece& piece, const ChessBoard& board, int target) {
    const int row_delta = target / BOARD_SIZE - piece.getRow();
    const int col_delta = target % BOARD_SIZE - piece.getCol();
    if (row_delta == 0 && col_delta == 0) return false;

    switch (piece.getKind()) {
    case PieceKind::pawn:
        return row_delta == (piece.getColor() == white ? -1 : 1) &&
               std::abs(col_delta) == 1;
    case PieceKind::knight:
        return (std::abs(row_delta) == 1 && std::abs(col_delta) == 2) ||
               (std::abs(row_delta) == 2 && std::abs(col_delta) == 1);
    case PieceKind::king:
        return std::abs(row_delta) <= 1 && std::abs(col_delta) <= 1;
    case PieceKind::bishop:
        if (std::abs(row_delta) != std::abs(col_delta)) return false;
        break;
    case PieceKind::rook:
        if (row_delta != 0 && col_delta != 0) return false;
        break;
    case PieceKind::queen:
        if (row_delta != 0 && col_delta != 0 &&
            std::abs(row_delta) != std::abs(col_delta)) return false;
        break;
    }

    const int row_step = (row_delta > 0) - (row_delta < 0);
    const int col_step = (col_delta > 0) - (col_delta < 0);
    for (int row = piece.getRow() + row_step, col = piece.getCol() + col_step;
         row != target / BOARD_SIZE || col != target % BOARD_SIZE;
         row += row_step, col += col_step) {
        if (board.pieceAt(board.Index(row, col))) return false;
    }
    return true;
}

int attackCount(const ChessBoard& board, int target, Color color) {
    int count = 0;
    for (int square = 0; square < NUM_SQUARES; ++square) {
        const Piece* piece = board.pieceAt(square);
        if (piece && piece->getColor() == color && attacksSquare(*piece, board, target)) {
            ++count;
        }
    }
    return count;
}

} // namespace

int Ai::evaluatePosition(const ChessBoard& board, Color perspective) const {
    int score = 0; // White's point of view; converted at the end.
    int white_mobility = 0;
    int black_mobility = 0;

    for (const auto& piece : board.pieces_) {
        if (!piece) continue;
        const Color color = piece->getColor();
        const int sign = color == white ? 1 : -1;
        const int row = color == white ? piece->getRow() : BOARD_SIZE - 1 - piece->getRow();
        const int index = board.Index(row, piece->getCol());
        score += sign * (piece->getPrice() + positionalBonus(piece->getKind(), index));

        const int mobility = static_cast<int>(piece->FINDPOSSIBLEMOVES(board).size());
        (color == white ? white_mobility : black_mobility) += mobility;

        if (board.history_.size() < 20 &&
            (piece->getKind() == PieceKind::knight || piece->getKind() == PieceKind::bishop) &&
            piece->getRow() != (color == white ? 7 : 0)) {
            score += sign * 20;
        }
    }

    score += (white_mobility - black_mobility) * 5;

    for (int square = 0; square < NUM_SQUARES; ++square) {
        const Piece* target = board.pieceAt(square);
        if (!target) continue;
        const Color attacker_color = target->getColor() == white ? black : white;
        const int attackers = attackCount(board, square, attacker_color);
        if (attackers == 0) continue;

        const int defenders = attackCount(board, square, target->getColor());
        const int value = target->getKind() == PieceKind::king ? 200 : target->getPrice();
        const int penalty = defenders == 0 ? value / 2
                          : attackers > defenders ? value / 4 : value / 10;
        score += target->getColor() == white ? -penalty : penalty;
    }

    return perspective == white ? score : -score;
}



const int PAWN_PST[NUM_SQUARES] = {
    0,   0,   0,   0,   0,   0,   0,   0,
    3,   7,   6,  12,   8,  10,   7,   3,
    14,  22,  32,  41,  44,  49,  33,  15,
    46,  78, 125, 288, 169,  75,  58,  38,
    100,119, 205, 336, 210, 110,  78,  85,
    112,119, 101,  32,  81,  91, 185, 147,
    55,  69,  93, 141, 102,  67,  72,  57,
    0,   0,   0,   0,   0,   0,   0,   0
};

const int KNIGHT_PST[NUM_SQUARES] = {
    290,	300,	300,	300,	300,	300,	300,	290,
    300,	305,	305,	305,	305,	305,	305,	300,
    300,	305,	325,	325,	325,	325,	305,	300,
    300,	305,	325,	325,	325,	325,	305,	300,
    300,	305,	325,	325,	325,	325,	305,	300,
    300,	305,	320,	325,	325,	325,	305,	300,
    300,	305,	305,	305,	305,	305,	305,	300,
    290,	310,	300,	300,	300,	300,	310,	290
};

const int BISHOP_PST[NUM_SQUARES] = {
    320,	320,	320,	320,	320,	320,	320,	320,
    322,	322,	322,	322,	322,	322,	322,	322,
    320,	322,	325,	325,	325,	325,	322,	320,
    320,	325,	330,	330,	330,	330,	325,	320,
    320,	330,	330,	330,	330,	330,	330,	320,
    320,	325,	325,	325,	325,	325,	325,	320,
    320,	320,	320,	320,	320,	320,	320,	320,
    320,	320,	320,	320,	320,	320,	320,	320
};

const int ROOK_PST[NUM_SQUARES] = {
	500,	500,	500,	500,	500,	500,	500,	500,
	520,	520,	520,	520,	520,	520,	520,	520,
	500,	500,	500,	500,	500,	500,	500,	500,
	500,	500,	500,	500,	500,	500,	500,	500,
	500,	500,	500,	500,	500,	500,	500,	500,
	500,	500,	500,	500,	500,	500,	500,	500,
	500,	500,	500,	500,	500,	500,	500,	500,
	500,	500,	500,	510,	510,	505,	500,	500
};

const int QUEEN_PST[NUM_SQUARES] = {
	980,	980,	990,	1000,	1000,	990,	980,	980,
	980,	990,	1000,	1010,	1010,	1000,	990,	980,
	990,	1000,	1010,	1020,	1020,	1010,	1000,	990,
	1000,	1010,	1020,	1030,	1030,	1020,	1010,	1000,
	1000,	1010,	1020,	1030,	1030,	1020,	1010,	1000,
	990,	1000,	1010,	1020,	1020,	1010,	1000,	990,
	980,	990,	1000,	1010,	1010,	1000,	990,	980,
	980,	980,	990,	1000,	1000,	990,	980,	980
};

const int KING_PST[NUM_SQUARES] = {
	-95,	-95,	-90,	-90,	-90,	-90,	-95,	-95,
	-95,	-50,	-50,	-50,	-50,	-50,	-50,	-95,
	-90,	-50,	-20,	-20,	-20,	-20,	-50,	-90,
	-90,	-50,	-20,	0,	    0,	    -20,	-50,	-90,
	-90,	-50,	-20,	0,	    0,	    -20,	-50,	-90,
	-90,	-50,	-20,	-20,	-20,    -20,	-50,	-90,
	-95,	-50,	-50,	-50,	-50,	-50,	-50,	-95,
	-95,	-95,	-90,	-90,	-90,	-90,	-95,	-95
};

} // namespace ce
