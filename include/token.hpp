#pragma once

#include <stdexcept>
#include <string>

enum class TokenType {
    Set,
    If,
    Else,
    While,
    Step,
    Turn,
    Mine,
    Print,
    IsThere,
    Plus,
    Minus,
    Multiply,
    Equals,
    NotEquals,
    LessThan,
    LessEqual,
    GreaterThan,
    GreaterEqual,
    And,
    Or,
    Not,
    BooleanLit,
    Ident,
    Number,
    LeftParen,
    RightParen,
    Newline,
    Indent,
    Dedent,
    End
};

struct Token {
    TokenType type;
    std::string value;
    int line;
};

[[noreturn]] void error(std::string message, int line);
