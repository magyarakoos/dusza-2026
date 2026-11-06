#include "token.hpp"

[[noreturn]] void error(std::string message, int line) {
    throw std::runtime_error(
        "Line " + std::to_string(line) + ": " + message);
}
