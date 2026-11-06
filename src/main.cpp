// #define FTXUI_IMPLEMENTATION
// #include "ftxui.hpp"
// using namespace ftxui;

#include "robot_env.hpp"

int main() {
    RobotEnv env("test/input_001.txt");
    env.run();

    // std::vector<std::string> entries = {"Option 1",
    //                                     "Option 2",
    //                                     "Exit"};
    //
    // int selected = 0;
    //
    // auto screen = ScreenInteractive::TerminalOutput();
    // auto menu = Menu(&entries, &selected);
    //
    // screen.Loop(menu);
}
