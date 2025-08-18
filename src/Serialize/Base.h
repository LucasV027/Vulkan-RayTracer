#pragma once

#include <glm/glm.hpp>

#include <nlohmann/json.hpp>
using Json = nlohmann::json;

#define Serializable(T)                     \
    friend void to_json(Json&, const T&);   \
    friend void from_json(const Json&, T&);

