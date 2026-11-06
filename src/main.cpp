// #define FTXUI_IMPLEMENTATION
// #include "ftxui.hpp"
// using namespace ftxui;

#include "robot_env.hpp"

int main() {
    RobotEnv env("test/input_002.txt");

    std::ofstream out("test/output_002.txt");
    while (auto line = env.step()) out << *line << '\n';
    out.close();

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
