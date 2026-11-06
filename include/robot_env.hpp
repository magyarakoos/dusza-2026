#pragma once

#include <fstream>
#include <iostream>

#include "ast.hpp"
#include "cave_map.hpp"
#include "interpreter.hpp"
#include "lexer.hpp"
#include "parser.hpp"

class RobotEnv {
    struct Loaded {
        CaveMap map;
        Program program;
        std::vector<std::string> program_lines;
    };

    static Loaded load(const std::string& path) {
        std::ifstream file(path);
        int height, width;
        file >> height >> width;

        std::string rest;
        std::getline(file, rest);

        std::vector<std::string> map_vec(height);
        for (int i = 0; i < height; i++) {
            std::getline(file, map_vec[i]);
        }

        std::vector<std::string> lines;
        std::string line;
        while (std::getline(file, line) && !line.empty()) {
            lines.push_back(line);
        }

        Lexer lexer(lines);
        auto tokens = lexer.tokenize();
        Parser parser(tokens);

        return {CaveMap(height, width, map_vec),
                parser.parse(), lines};
    }

    RobotEnv(Loaded&& l)
        : map(std::move(l.map)),
          program(std::move(l.program)),
          interpreter(program, map) {}

  public:
    CaveMap map;
    Program program;
    Interpreter interpreter;

    std::vector<std::string> program_lines;

    RobotEnv(const std::string& path)
        : RobotEnv(load(path)) {}

    StepOutput step() { return interpreter.step(); }
};
