#pragma once

#include <string>

struct RunError {
    std::string code;
};

namespace err {
inline const std::string TypeMismatch = "E_TYPE_MISMATCH";
inline const std::string Uninitialized = "E_UNINITIALIZED";
inline const std::string NonBoolCondition =
    "E_NON_BOOL_CONDITION";
inline const std::string BadOperand = "E_BAD_OPERAND";
inline const std::string BedrockHit = "E_BEDROCK_HIT";
inline const std::string NoGold = "E_NO_GOLD";
inline const std::string BadArg = "E_BAD_ARG";
inline const std::string LimitExceeded =
    "INSTRUCTION_COUNT_LIMIT_EXCEEDED";
} // namespace err
