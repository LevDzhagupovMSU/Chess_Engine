#include "serializer.hpp"
#include "move_interface.hpp"
#include "player_move.cpp"
#include "ai_move.cpp"
#include "bad_move.cpp"
#include <memory>
#include <string>

namespace {

const char* promotionName(ce::PieceKind kind) {
    switch (kind) {
    case ce::PieceKind::rook: return "rook";
    case ce::PieceKind::bishop: return "bishop";
    case ce::PieceKind::knight: return "knight";
    default: return "queen";
    }
}

bool parsePromotion(const nlohmann::json& j, ce::PieceKind& result) {
    result = ce::PieceKind::queen;
    if (!j.contains("promotion")) return true;
    if (!j["promotion"].is_string()) return false;
    const auto name = j["promotion"].get<std::string>();
    if (name == "queen") result = ce::PieceKind::queen;
    else if (name == "rook") result = ce::PieceKind::rook;
    else if (name == "bishop") result = ce::PieceKind::bishop;
    else if (name == "knight") result = ce::PieceKind::knight;
    else return false;
    return true;
}

} // namespace

nlohmann::json Serializer::to_json(const IMove& move) {
    nlohmann::json j;
    if(dynamic_cast<const Player_move*>(&move)) {
        j["type"] = "player";
    } else if(dynamic_cast<const Ai_move*>(&move)) {
        j["type"] = "ai";
    }else if(dynamic_cast<const bad_move*>(&move)) {
        j["type"] = "bad move";
    }
    j["move"] = {move.move.from, move.move.to};
    if (move.move.promotion != ce::PieceKind::queen) {
        j["promotion"] = promotionName(move.move.promotion);
    }
    return j;
}

std::unique_ptr<IMove> Serializer::from_json(const std::string& message) {
    try {
        return from_json(nlohmann::json::parse(message));
    } catch (const std::exception&) {
        return std::make_unique<bad_move>();
    }
}

std::unique_ptr<IMove> Serializer::from_json(const nlohmann::json& j){
    if (!j.is_object() || !j.contains("type") || !j["type"].is_string() ||
        !j.contains("move") || !j["move"].is_array() || j["move"].size() != 2 ||
        !j["move"][0].is_number_integer() || !j["move"][1].is_number_integer()) {
        return std::make_unique<bad_move>();
    }

    ce::PieceKind promotion;
    if (!parsePromotion(j, promotion)) return std::make_unique<bad_move>();
    const ce::Move move{j["move"][0].get<int>(), j["move"][1].get<int>(), promotion};
    const auto type = j["type"].get<std::string>();
    if(type == "player"){
        return std::make_unique<Player_move>(move);
    }else if(type == "ai"){
        return std::make_unique<Ai_move>(move);
    } else{
        return std::make_unique<bad_move>();
    }
}

std::optional<ce::Move> Serializer::playerMoveFromJson(const std::string& message) {
    auto parsed = from_json(message);
    if (!dynamic_cast<Player_move*>(parsed.get())) return std::nullopt;
    return parsed->move;
}

nlohmann::json Serializer::aiMoveToJson(ce::Move move) {
    return to_json(Ai_move(move));
}

nlohmann::json Serializer::badMoveToJson() {
    return to_json(bad_move());
}
