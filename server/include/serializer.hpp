#pragma once

#include "move_interface.hpp"
#include <nlohmann/json.hpp>
#include <memory>
#include <optional>
#include <string>

struct Serializer{
    static nlohmann::json to_json(const IMove& move);

    static std::unique_ptr<IMove> from_json(const nlohmann::json& j);
    static std::unique_ptr<IMove> from_json(const std::string& message);
    static std::optional<ce::Move> playerMoveFromJson(const std::string& message);
    static nlohmann::json aiMoveToJson(ce::Move move);
    static nlohmann::json badMoveToJson();
};
