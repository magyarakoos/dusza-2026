#pragma once

#include <deque>
#include <optional>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

#include "ast.hpp"
#include "cave_map.hpp"
#include "errors.hpp"

using Value =
    std::variant<long long, bool>; // integer | boolean

// API names exactly as written in the source code
namespace api {
inline const std::string IsThere = "isThere";
inline const std::string Step = "step forward";
inline const std::string Turn = "turn by";
inline const std::string Mine = "mine here";
inline const std::string Print = "print";
} // namespace api

class Interpreter {
    static constexpr int MAX_LINES = 1000;

    // A block that is currently being executed.
    struct Frame {
        const std::vector<Stmt>* body;
        size_t pc = 0;
        int pendingLine =
            0; // `else` line still to be logged (0 = none)
    };

    // One log entry: line [api [param] [return]] [error]
    struct LineLog {
        int line = 0;
        std::string api;
        std::optional<std::string> param;
        std::optional<std::string> ret;
        std::string error;

        std::string str() const {
            std::string s = std::to_string(line);
            if (!api.empty()) s += " " + api;
            if (param) s += " " + *param;
            if (ret) s += " " + *ret;
            if (!error.empty()) s += " " + error;
            return s;
        }
    };

    enum class Type { Int, Bool };

    const Program* program;
    CaveMap map;
    std::deque<Frame> stack;
    LineLog cur;
    int processed = 0;
    bool halted = false; // a runtime error happened -> next
                         // step() is EOR
    bool finished = false; // the last entry (EOR / limit)
                           // was already returned

    // ---------- helpers ----------
    static bool isInt(const Value& v) {
        return std::holds_alternative<long long>(v);
    }
    static bool isBool(const Value& v) {
        return std::holds_alternative<bool>(v);
    }

    static std::string fmt(const Value& v) {
        if (isBool(v))
            return std::get<bool>(v) ? "true" : "false";
        return std::to_string(std::get<long long>(v));
    }

    // Checks the operand type against what the operation
    // needs, right now.
    static void require(const Value& v, Type t) {
        if (t == Type::Int ? !isInt(v) : !isBool(v))
            throw RunError{err::BadOperand};
    }

    // Evaluates an API argument and logs it. If the
    // evaluation itself fails (e.g. uninitialized variable)
    // the param stays out of the log: "isThere
    // E_UNINITIALIZED". A wrong-typed argument is logged
    // and then gives E_BAD_ARG ("turn by true E_BAD_ARG").
    long long apiArg(const std::string& name,
                     const ExprPtr& e) {
        cur.api = name;
        Value v = evaluate(e);
        cur.param = fmt(v);
        if (!isInt(v)) throw RunError{err::BadArg};
        return std::get<long long>(v);
    }

    // ---------- expressions ----------
    Value evaluate(const ExprPtr& expr) {
        return std::visit(
            [&](const auto& node) -> Value {
                using T = std::decay_t<decltype(node)>;

                if constexpr (std::is_same_v<T,
                                             NumberExpr>) {
                    return Value(
                        static_cast<long long>(node.value));
                } else if constexpr (std::is_same_v<
                                         T, BooleanExpr>) {
                    return Value(
                        static_cast<bool>(node.value));
                } else if constexpr (std::is_same_v<
                                         T, VariableExpr>) {
                    auto it = variables.find(node.name);
                    if (it == variables.end())
                        throw RunError{err::Uninitialized};
                    return it->second;
                } else if constexpr (std::is_same_v<
                                         T, IsThereExpr>) {
                    long long w =
                        apiArg(api::IsThere, node.argument);
                    bool r = map.isThere(
                        w); // E_BAD_ARG if w is not 0, 1, 2
                    cur.ret = r ? "true" : "false";
                    return Value(r);
                } else if constexpr (std::is_same_v<
                                         T, UnaryExpr>) {
                    // the only unary operator is `not`
                    Value right = evaluate(node.right);
                    require(right, Type::Bool);
                    return Value(!std::get<bool>(right));
                } else if constexpr (std::is_same_v<
                                         T, BinaryExpr>) {
                    const TokenType op = node.op;
                    const bool isEq =
                        op == TokenType::Equals ||
                        op == TokenType::NotEquals;
                    const bool isLogic =
                        op == TokenType::And ||
                        op == TokenType::Or;
                    const Type need =
                        isLogic ? Type::Bool : Type::Int;

                    // Operands are read left to right, each
                    // one checked immediately. No
                    // short-circuit: `true or c` still
                    // reads c.
                    Value left = evaluate(node.left);
                    if (!isEq) require(left, need);

                    Value right = evaluate(node.right);
                    if (isEq) {
                        // type match is checked after
                        // reading the 2nd operand
                        if (left.index() != right.index())
                            throw RunError{err::BadOperand};
                        bool eq = (left == right);
                        return Value(op == TokenType::Equals
                                         ? eq
                                         : !eq);
                    }
                    require(right, need);

                    if (isLogic) {
                        bool a = std::get<bool>(left),
                             b = std::get<bool>(right);
                        return Value(op == TokenType::And
                                         ? (a && b)
                                         : (a || b));
                    }

                    long long a = std::get<long long>(left);
                    long long b =
                        std::get<long long>(right);
                    switch (op) {
                    case TokenType::Plus:
                        return Value(a + b);
                    case TokenType::Minus:
                        return Value(a - b);
                    case TokenType::Multiply:
                        return Value(a * b);
                    case TokenType::LessThan:
                        return Value(a < b);
                    case TokenType::LessEqual:
                        return Value(a <= b);
                    case TokenType::GreaterThan:
                        return Value(a > b);
                    case TokenType::GreaterEqual:
                        return Value(a >= b);
                    default:
                        throw RunError{err::BadOperand};
                    }
                } else {
                    throw RunError{err::BadOperand};
                }
            },
            expr->node);
    }

    bool condition(const ExprPtr& e) {
        Value v = evaluate(e);
        if (!isBool(v))
            throw RunError{err::NonBoolCondition};
        return std::get<bool>(v);
    }

    // ---------- statements ----------
    // Executes ONE line. Bodies are not run here, only
    // pushed on the stack.
    void execute(const Stmt& stmt, Frame& f) {
        std::visit(
            [&](const auto& node) {
                using T = std::decay_t<decltype(node)>;

                if constexpr (std::is_same_v<T, SetStmt>) {
                    f.pc++;
                    Value v = evaluate(
                        node.value); // first evaluate...
                    auto it = variables.find(node.name);
                    if (it != variables.end() &&
                        it->second.index() != v.index())
                        throw RunError{err::TypeMismatch};
                    variables[node.name] =
                        v; // ...then store
                } else if constexpr (std::is_same_v<
                                         T, IfStmt>) {
                    f.pc++;
                    if (condition(node.condition))
                        stack.push_back({&node.body, 0, 0});
                    else if (stmt.line +
                                 (int)node.body.size() + 1 >
                             0) // else line is logged next
                                // step
                        stack.push_back(
                            {&node.elseBody, 0,
                             stmt.line +
                                 (int)node.body.size() +
                                 1});
                } else if constexpr (std::is_same_v<
                                         T, WhileStmt>) {
                    // pc stays on the while line while the
                    // condition holds, so after the body
                    // the condition is evaluated (and
                    // logged) again.
                    if (condition(node.condition))
                        stack.push_back({&node.body, 0, 0});
                    else
                        f.pc++;
                } else if constexpr (std::is_same_v<
                                         T, StepStmt>) {
                    f.pc++;
                    map.stepForward(
                        apiArg(api::Step, node.distance));
                } else if constexpr (std::is_same_v<
                                         T, TurnStmt>) {
                    f.pc++;
                    map.turnLeft(
                        apiArg(api::Turn, node.angle));
                } else if constexpr (std::is_same_v<
                                         T, MineStmt>) {
                    f.pc++;
                    cur.api = api::Mine;
                    map.mine();
                } else if constexpr (std::is_same_v<
                                         T, PrintStmt>) {
                    f.pc++;
                    cur.api = api::Print;
                    Value v = evaluate(node.value);
                    cur.param = fmt(v);
                    console.push_back(*cur.param);
                }
            },
            stmt.node);
    }

    void skipFinishedBlocks() {
        while (!stack.empty() &&
               stack.back().pendingLine == 0 &&
               stack.back().pc >= stack.back().body->size())
            stack.pop_back();
    }

    std::string finish(const std::string& last) {
        finished = true;
        return last;
    }

  public:
    std::unordered_map<std::string, Value>
        variables; // for the debugger view
    std::vector<std::string> console; // output of `print`

    Interpreter(const Program& p, CaveMap m)
        : program(&p), map(std::move(m)) {
        stack.push_back({program, 0, 0});
    }

    bool isFinished() const { return finished; }
    const CaveMap& cave() const { return map; }

    // Source line that the next step() will process (0 =
    // none). For highlighting the current line while
    // debugging.
    int nextLine() {
        if (finished || halted) return 0;
        skipFinishedBlocks();
        if (stack.empty()) return 0;
        const Frame& f = stack.back();
        return f.pendingLine ? f.pendingLine
                             : (*f.body)[f.pc].line;
    }

    struct StepOutput {
        std::optional<std::string> log;
        std::optional<std::string> print;
    };

    // Processes one program line and returns its log entry.
    // The last entry is "EOR" (or
    // INSTRUCTION_COUNT_LIMIT_EXCEEDED, with no EOR after
    // it); after that step() returns std::nullopt.
    StepOutput step() {
        const int console_size = console.size();

        if (finished) return {std::nullopt, std::nullopt};
        if (halted) return {finish("EOR"), std::nullopt};

        skipFinishedBlocks();
        if (stack.empty())
            return {finish("EOR"), std::nullopt};

        // another line is due: 1000-line limit
        if (processed >= MAX_LINES)
            return {finish(err::LimitExceeded),
                    std::nullopt};
        processed++;

        Frame& f = stack.back();
        cur = LineLog{};

        // `else` line: logged on its own, its body runs on
        // the next steps
        if (f.pendingLine != 0) {
            cur.line = f.pendingLine;
            f.pendingLine = 0;
            return {cur.str(), std::nullopt};
        }

        const Stmt& stmt = (*f.body)[f.pc];
        cur.line = stmt.line;

        try {
            execute(stmt, f);
        } catch (const RunError& e) {
            cur.error = e.code;
            halted = true; // an error ends the run
        }

        if (console.size() > console_size) {
            return {cur.str(), console.back()};
        }
        return {cur.str(), std::nullopt};
    }
};
