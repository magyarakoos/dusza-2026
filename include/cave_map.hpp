#pragma once

#include <stdexcept>
#include <string>
#include <vector>

#include "errors.hpp"

class CaveMap {
    int height = 0;
    int width = 0;

  public:
    // isThere codes: 0 = empty, 1 = wall, 2 = gold
    enum Kind { Empty = 0, Wall = 1, Gold = 2 };

    static constexpr char C_EMPTY = '.';
    static constexpr char C_GOLD = '@';
    static constexpr char C_WALL = '#';

    std::vector<std::string> map;
    std::vector<std::vector<bool>>
        visited; // explored cells
    int robot_i = -1;
    int robot_j = -1;
    int robot_dir = -1; // index into dirs
    int minedCount = 0;

    const std::string dirs =
        "^>v<"; // north, east, south, west

    CaveMap() = default;

    CaveMap(int _height, int _width,
            const std::vector<std::string>& _map)
        : height(_height), width(_width), map(_map) {

        visited.assign(height,
                       std::vector<bool>(width, false));

        bool found = false;
        for (int i = 0; i < height && !found; i++) {
            for (int j = 0; j < width && !found; j++) {
                size_t index = dirs.find(map[i][j]);
                if (index != std::string::npos) {
                    robot_i = i;
                    robot_j = j;
                    robot_dir = (int)index;
                    map[i][j] = C_EMPTY; // the robot never
                                         // starts on gold
                    visited[i][j] =
                        true; // the robot has been here
                    found = true;
                }
            }
        }
        if (!found)
            throw std::runtime_error(
                "Robot not on cave map");
    }

    int getHeight() const { return height; }
    int getWidth() const { return width; }

    // ---------- queries ----------
    bool inBounds(int i, int j) const {
        return i >= 0 && j >= 0 && i < height && j < width;
    }

    // Outside of the map counts as wall.
    Kind kindAt(int i, int j) const {
        if (!inBounds(i, j)) return Wall;
        char c = map[i][j];
        if (c == C_WALL) return Wall;
        if (c == C_GOLD) return Gold;
        return Empty;
    }

    static constexpr int di[4] = {-1, 0, 1, 0};
    static constexpr int dj[4] = {0, 1, 0, -1};

    int aheadI() const { return robot_i + di[robot_dir]; }
    int aheadJ() const { return robot_j + dj[robot_dir]; }

    // What the sensor shows right now (what isThere would
    // answer). Does NOT mark the cell as explored.
    Kind ahead() const {
        return kindAt(aheadI(), aheadJ());
    }

    // ---------- robot API (used by the program, throws
    // RunError) ----------

    // isThere w: w must be 0, 1 or 2. Marks the cell in
    // front as explored.
    bool isThere(long long w) {
        if (w < 0 || w > 2) throw RunError{err::BadArg};
        if (inBounds(aheadI(), aheadJ()))
            visited[aheadI()][aheadJ()] = true;
        return (long long)ahead() == w;
    }

    // turn by n: n >= 0 quarter turns to the LEFT
    // (counter-clockwise).
    void turnLeft(long long n) {
        if (n < 0) throw RunError{err::BadArg};
        robot_dir =
            (int)(((robot_dir - n % 4) % 4 + 4) % 4);
    }

    // step forward n: n >= 0. Touches every cell on the
    // way, leaves gold where it is. If the next cell is
    // wall / off the map, the robot stays on the last valid
    // cell and E_BEDROCK_HIT is raised.
    void stepForward(long long n) {
        if (n < 0) throw RunError{err::BadArg};
        for (long long k = 0; k < n; k++) {
            if (ahead() == Wall)
                throw RunError{err::BedrockHit};
            robot_i = aheadI();
            robot_j = aheadJ();
            visited[robot_i][robot_j] = true;
        }
    }

    // mine here: mines the gold in the cell the robot
    // stands on.
    void mine() {
        if (map[robot_i][robot_j] != C_GOLD)
            throw RunError{err::NoGold};
        map[robot_i][robot_j] = C_EMPTY;
        minedCount++;
    }

    // ---------- manual control (no exceptions, no logging)
    // ---------- Hitting a wall simply stops the robot.
    bool manualStep() {
        if (ahead() == Wall) return false;
        robot_i = aheadI();
        robot_j = aheadJ();
        visited[robot_i][robot_j] = true;
        return true;
    }
    void manualTurnLeft() {
        robot_dir = (robot_dir + 3) % 4;
    }
    void manualTurnRight() {
        robot_dir = (robot_dir + 1) % 4;
    }
    bool manualMine() {
        if (map[robot_i][robot_j] != C_GOLD) return false;
        map[robot_i][robot_j] = C_EMPTY;
        minedCount++;
        return true;
    }
};
