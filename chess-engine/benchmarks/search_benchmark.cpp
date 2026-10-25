#include "game.hpp"

#include <algorithm>
#include <chrono>
#include <climits>
#include <iomanip>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace {

volatile int benchmark_sink = 0;

int square(char file, int rank) {
    return (8 - rank) * ce::BOARD_SIZE + (file - 'a');
}

ce::Move move(char from_file, int from_rank, char to_file, int to_rank) {
    return {square(from_file, from_rank), square(to_file, to_rank)};
}

void play(ce::Game& game, ce::Move next) {
    if (!game.makeMove(next)) throw std::runtime_error("Invalid benchmark position");
}

ce::Game opening() {
    ce::Game game;
    play(game, move('e', 2, 'e', 4));
    return game;
}

ce::Game middleGame() {
    ce::Game game;
    play(game, move('e', 2, 'e', 4));
    play(game, move('e', 7, 'e', 5));
    play(game, move('g', 1, 'f', 3));
    play(game, move('b', 8, 'c', 6));
    play(game, move('f', 1, 'c', 4));
    play(game, move('f', 8, 'c', 5));
    play(game, move('c', 2, 'c', 3));
    play(game, move('g', 8, 'f', 6));
    return game;
}

ce::Game endgame() {
    ce::ChessBoard board;
    board.clear();
    board.setPiece(7, 0, std::make_unique<ce::King>(7, 0, 50000, ce::white));
    board.setPiece(0, 7, std::make_unique<ce::King>(0, 7, 50000, ce::black));
    board.setPiece(4, 3, std::make_unique<ce::Queen>(4, 3, 900, ce::white));
    board.setPiece(3, 3, std::make_unique<ce::Pawn>(3, 3, 100, ce::black));
    board.setPiece(0, 3, std::make_unique<ce::Rook>(0, 3, 500, ce::black));
    return ce::Game(std::move(board));
}

int parseNonnegative(const std::string& value) {
    std::size_t parsed = 0;
    const long number = std::stol(value, &parsed);
    if (parsed != value.size() || number < 0 || number > INT_MAX) {
        throw std::invalid_argument("Expected a nonnegative integer: " + value);
    }
    return static_cast<int>(number);
}

double median(const std::vector<double>& sorted) {
    const std::size_t middle = sorted.size() / 2;
    return sorted.size() % 2 ? sorted[middle]
                             : (sorted[middle - 1] + sorted[middle]) / 2.0;
}

void run(const char* name, ce::Game& game, int depth,
         int iterations, int warmup) {
    for (int i = 0; i < warmup; ++i) {
        const ce::Move selected = game.getAiMove(depth);
        benchmark_sink += selected.from + selected.to;
    }

    std::vector<double> samples;
    samples.reserve(static_cast<std::size_t>(iterations));
    ce::Move selected{-1, -1};
    for (int i = 0; i < iterations; ++i) {
        const auto start = std::chrono::steady_clock::now();
        const ce::Move current = game.getAiMove(depth);
        const auto end = std::chrono::steady_clock::now();
        if (i == 0) selected = current;
        if (current.from != selected.from || current.to != selected.to ||
            current.promotion != selected.promotion) {
            throw std::runtime_error("Search returned inconsistent moves");
        }
        benchmark_sink += current.from + current.to;
        samples.push_back(std::chrono::duration<double, std::milli>(end - start).count());
    }
    ce::Game verification(game);
    if (!verification.makeMove(selected)) {
        throw std::runtime_error("Search returned an illegal move");
    }

    std::sort(samples.begin(), samples.end());
    std::cout << name << ',' << depth << ',' << iterations << ','
              << median(samples) << ',' << samples.front() << ',' << samples.back()
              << ',' << selected.from << ',' << selected.to << ','
              << static_cast<int>(selected.promotion) << '\n';
}

} // namespace

int main(int argc, char** argv) {
    try {
        int iterations = 5;
        int warmup = 1;
        std::vector<int> depths{1, 2};
        bool custom_depths = false;
        for (int i = 1; i < argc; ++i) {
            const std::string option = argv[i];
            if (option == "--help") {
                std::cout << "Usage: chess_engine_search_benchmark "
                             "[--iterations N] [--warmup N] [--depth N ...]\n";
                return 0;
            }
            if (i + 1 >= argc) throw std::invalid_argument("Missing value for " + option);
            const int value = parseNonnegative(argv[++i]);
            if (option == "--iterations") {
                if (value == 0) throw std::invalid_argument("Iterations must be positive");
                iterations = value;
            } else if (option == "--warmup") {
                warmup = value;
            } else if (option == "--depth") {
                if (value == 0) throw std::invalid_argument("Depth must be positive");
                if (!custom_depths) {
                    depths.clear();
                    custom_depths = true;
                }
                depths.push_back(value);
            } else {
                throw std::invalid_argument("Unknown option: " + option);
            }
        }

        std::cout << std::fixed << std::setprecision(3);
        std::cout << "scenario,depth,iterations,median_ms,min_ms,max_ms,"
                     "move_from,move_to,promotion\n";
        for (int depth : depths) {
            ce::Game opening_game = opening();
            ce::Game middle_game = middleGame();
            ce::Game end_game = endgame();
            run("opening", opening_game, depth, iterations, warmup);
            run("middle_game", middle_game, depth, iterations, warmup);
            run("endgame", end_game, depth, iterations, warmup);
        }
    } catch (const std::exception& error) {
        std::cerr << "Benchmark failed: " << error.what() << '\n';
        return 1;
    }
}
