#pragma once

#include <memory>
#include <variant>
#include <vector>

#include "token.hpp"

struct Expr;
using ExprPtr = std::shared_ptr<Expr>;

struct NumberExpr {
    double value;
};
struct BooleanExpr {
    bool value;
};
struct VariableExpr {
    std::string name;
};

struct UnaryExpr {
    TokenType op;
    ExprPtr right;
};

struct BinaryExpr {
    ExprPtr left;
    TokenType op;
    ExprPtr right;
};

struct IsThereExpr {
    ExprPtr argument;
};

struct Expr {
    std::variant<NumberExpr, BooleanExpr, VariableExpr,
                 UnaryExpr, BinaryExpr, IsThereExpr>
        node;
};

struct Stmt;

struct SetStmt {
    std::string name;
    ExprPtr value;
};

struct IfStmt {
    ExprPtr condition;
    std::vector<Stmt> body;
    std::vector<Stmt> elseBody;
};

struct WhileStmt {
    ExprPtr condition;
    std::vector<Stmt> body;
};

struct StepStmt {
    ExprPtr distance;
};
struct TurnStmt {
    ExprPtr angle;
};
struct MineStmt {};
struct PrintStmt {
    ExprPtr value;
};

struct Stmt {
    std::variant<SetStmt, IfStmt, WhileStmt, StepStmt,
                 TurnStmt, MineStmt, PrintStmt>
        node;
    int line;
};

using Program = std::vector<Stmt>;
