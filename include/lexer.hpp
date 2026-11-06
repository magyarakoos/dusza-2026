#pragma once

#include <string>
#include <unordered_map>
#include <vector>

#include "token.hpp"

class Lexer {
    std::vector<std::string> source;
    std::vector<Token> tokens;
    std::vector<int> indents = {0};

    void emit(TokenType type, std::string value, int line) {
        tokens.push_back({type, value, line});
    }

    TokenType keyword(const std::string& s) {
        static const std::unordered_map<std::string,
                                        TokenType>
            keywords = {{"set", TokenType::Set},
                        {"if", TokenType::If},
                        {"else", TokenType::Else},
                        {"while", TokenType::While},
                        {"step", TokenType::Step},
                        {"turn", TokenType::Turn},
                        {"mine", TokenType::Mine},
                        {"print", TokenType::Print},
                        {"isThere", TokenType::IsThere},
                        {"and", TokenType::And},
                        {"or", TokenType::Or},
                        {"not", TokenType::Not},
                        {"true", TokenType::BooleanLit},
                        {"false", TokenType::BooleanLit}};

        auto it = keywords.find(s);
        return it == keywords.end() ? TokenType::Ident
                                    : it->second;
    }

  public:
    explicit Lexer(std::vector<std::string> src)
        : source(std::move(src)) {}

    std::vector<Token> tokenize() {
        for (int ln = 0; ln < (int)source.size(); ln++) {
            const std::string& line = source[ln];

            int spaces = 0;
            while (spaces < (int)line.size() &&
                   line[spaces] == ' ')
                spaces++;

            if (spaces == (int)line.size()) continue;

            if (line.find('\t') != std::string::npos)
                error("Tabs are not supported", ln + 1);

            if (spaces % 4 != 0)
                error("Indentation must use multiples of 4 "
                      "spaces",
                      ln + 1);

            if (spaces > indents.back()) {
                indents.push_back(spaces);
                emit(TokenType::Indent, "", ln + 1);
            } else {
                while (spaces < indents.back()) {
                    indents.pop_back();
                    emit(TokenType::Dedent, "", ln + 1);
                }

                if (spaces != indents.back())
                    error("Inconsistent indentation",
                          ln + 1);
            }

            for (int i = spaces; i < (int)line.size();) {
                char c = line[i];

                if (isspace((unsigned char)c)) {
                    i++;
                    continue;
                }

                if (isalpha((unsigned char)c) || c == '_') {
                    int start = i++;

                    while (
                        i < (int)line.size() &&
                        (isalnum((unsigned char)line[i]) ||
                         line[i] == '_'))
                        i++;

                    std::string word =
                        line.substr(start, i - start);
                    emit(keyword(word), word, ln + 1);
                    continue;
                }

                if (isdigit((unsigned char)c)) {
                    int start = i++;

                    while (i < (int)line.size() &&
                           isdigit((unsigned char)line[i]))
                        i++;

                    emit(TokenType::Number,
                         line.substr(start, i - start),
                         ln + 1);
                    continue;
                }

                if (i + 1 < (int)line.size()) {
                    std::string op = line.substr(i, 2);

                    if (op == "<>") {
                        emit(TokenType::NotEquals, op,
                             ln + 1);
                        i += 2;
                        continue;
                    }
                    if (op == "<=") {
                        emit(TokenType::LessEqual, op,
                             ln + 1);
                        i += 2;
                        continue;
                    }
                    if (op == ">=") {
                        emit(TokenType::GreaterEqual, op,
                             ln + 1);
                        i += 2;
                        continue;
                    }
                }

                TokenType type;

                switch (c) {
                case '+': type = TokenType::Plus; break;
                case '-': type = TokenType::Minus; break;
                case '*': type = TokenType::Multiply; break;
                case '=': type = TokenType::Equals; break;
                case '<': type = TokenType::LessThan; break;
                case '>':
                    type = TokenType::GreaterThan;
                    break;
                case '(':
                    type = TokenType::LeftParen;
                    break;
                case ')':
                    type = TokenType::RightParen;
                    break;
                default:
                    error("Unexpected character: " +
                              std::string(1, c),
                          ln + 1);
                }

                emit(type, std::string(1, c), ln + 1);
                i++;
            }

            emit(TokenType::Newline, "", ln + 1);
        }

        while (indents.size() > 1) {
            indents.pop_back();
            emit(TokenType::Dedent, "", source.size());
        }

        emit(TokenType::End, "", source.size());
        return tokens;
    }
};
