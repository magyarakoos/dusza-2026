#pragma once

#include <fstream>
#include <vector>

#include "ast.hpp"
#include "cave_map.hpp"
#include "token.hpp"

class Parser {
    std::vector<Token> tokens;
    size_t pos = 0;

    Token& peek() { return tokens[pos]; }

    Token advance() { return tokens[pos++]; }

    template <typename T>
    Stmt makeStmt(T node, int line) {
        return {std::move(node), line};
    }

    bool check(TokenType type) {
        return peek().type == type;
    }

    bool match(TokenType type) {
        if (!check(type)) return false;
        advance();
        return true;
    }

    Token consume(TokenType type, std::string message) {
        if (!check(type)) error(message, peek().line);
        return advance();
    }

    template <typename T>
    ExprPtr makeExpr(T node) {
        auto expr = std::make_shared<Expr>();
        expr->node = std::move(node);
        return expr;
    }

    ExprPtr binary(ExprPtr left, TokenType op,
                   ExprPtr right) {
        return makeExpr(BinaryExpr{left, op, right});
    }

    // expression -> or

    ExprPtr expression() {
        auto expr = parseUnary();

        while (isBinaryOperator(peek().type)) {
            TokenType op = advance().type;
            auto right = parseUnary();

            expr = binary(expr, op, right);
        }

        return expr;
    }

    bool isBinaryOperator(TokenType type) {
        switch (type) {
        case TokenType::Or:
        case TokenType::And:
        case TokenType::Equals:
        case TokenType::NotEquals:
        case TokenType::LessThan:
        case TokenType::LessEqual:
        case TokenType::GreaterThan:
        case TokenType::GreaterEqual:
        case TokenType::Plus:
        case TokenType::Minus:
        case TokenType::Multiply: return true;

        default: return false;
        }
    }

    ExprPtr parseUnary() {
        if (match(TokenType::Not))
            return makeExpr(
                UnaryExpr{TokenType::Not, parseUnary()});

        if (match(TokenType::Minus))
            return makeExpr(
                UnaryExpr{TokenType::Minus, parseUnary()});

        return parsePrimary();
    }

    ExprPtr parsePrimary() {
        if (match(TokenType::Number))
            return makeExpr(
                NumberExpr{stod(tokens[pos - 1].value)});

        if (match(TokenType::BooleanLit))
            return makeExpr(BooleanExpr{
                tokens[pos - 1].value == "true"});

        if (match(TokenType::Ident))
            return makeExpr(
                VariableExpr{tokens[pos - 1].value});

        if (match(TokenType::IsThere))
            return makeExpr(IsThereExpr{parsePrimary()});

        if (match(TokenType::LeftParen)) {
            auto expr = expression();
            consume(TokenType::RightParen, "Expected ')'");
            return expr;
        }

        error("Expected expression", peek().line);
    }

    // block -> INDENT { command } DEDENT

    std::vector<Stmt> parseBlock() {
        consume(TokenType::Newline,
                "Expected newline after block declaration");
        consume(TokenType::Indent,
                "Expected indented block");

        std::vector<Stmt> body;

        while (!check(TokenType::Dedent) &&
               !check(TokenType::End))
            body.push_back(parseStatement());

        consume(TokenType::Dedent, "Expected end of block");
        return body;
    }

    Stmt parseStatement() {
        int line = peek().line;

        if (match(TokenType::Set)) {
            std::string name =
                consume(TokenType::Ident,
                        "Expected variable name")
                    .value;

            consume(TokenType::Ident, "Expected 'to'");
            if (tokens[pos - 1].value != "to")
                error("Expected 'to'",
                      tokens[pos - 1].line);

            auto value = expression();
            consume(TokenType::Newline,
                    "Expected end of line");

            return makeStmt(SetStmt{name, value}, line);
        }

        if (match(TokenType::If)) {
            auto condition = expression();
            auto body = parseBlock();

            std::vector<Stmt> elseBody;

            if (match(TokenType::Else))
                elseBody = parseBlock();

            return makeStmt(IfStmt{condition,
                                   std::move(body),
                                   std::move(elseBody)},
                            line);
        }

        if (match(TokenType::While)) {
            auto condition = expression();
            auto body = parseBlock();

            return makeStmt(
                WhileStmt{condition, std::move(body)},
                line);
        }

        if (match(TokenType::Step)) {
            consume(TokenType::Ident, "Expected 'forward'");
            if (tokens[pos - 1].value != "forward")
                error("Expected 'forward'",
                      tokens[pos - 1].line);

            auto distance = expression();
            consume(TokenType::Newline,
                    "Expected end of line");

            return makeStmt(StepStmt{distance}, line);
        }

        if (match(TokenType::Turn)) {
            consume(TokenType::Ident, "Expected 'by'");
            if (tokens[pos - 1].value != "by")
                error("Expected 'by'",
                      tokens[pos - 1].line);

            auto angle = expression();
            consume(TokenType::Newline,
                    "Expected end of line");

            return makeStmt(TurnStmt{angle}, line);
        }

        if (match(TokenType::Mine)) {
            consume(TokenType::Ident, "Expected 'here'");
            if (tokens[pos - 1].value != "here")
                error("Expected 'here'",
                      tokens[pos - 1].line);

            consume(TokenType::Newline,
                    "Expected end of line");

            return makeStmt(MineStmt{}, line);
        }

        if (match(TokenType::Print)) {
            auto value = expression();
            consume(TokenType::Newline,
                    "Expected end of line");

            return makeStmt(PrintStmt{value}, line);
        }

        error("Expected command", peek().line);
    }

  public:
    explicit Parser(std::vector<Token> t)
        : tokens(std::move(t)) {}

    std::vector<Stmt> parse() {
        std::vector<Stmt> program;

        while (!check(TokenType::End)) {
            if (check(TokenType::Indent))
                error("Unexpected indentation",
                      peek().line);

            program.push_back(parseStatement());
        }

        return program;
    }
};
