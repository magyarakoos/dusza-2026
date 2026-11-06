#include <iostream>
#include "ftxui.hpp"
using namespace ftxui;
void kepernyo() {
    auto screen = ScreenInteractive::Fullscreen();
    screen.TrackMouse(true);  // Ensure mouse events are captured

    int left_column_width = 20;
    int top_window_height  = 10;

    auto top_left = Renderer([] {
        return window(text("Top Left"), text("Drag the border below me"));
    });

    auto bottom_left = Renderer([] {
        return window(text("Bottom Left"), text("Drag the border above me"));
    });

    auto right_window = Renderer([] {
        return window(text("Right"), text("Drag the border on my left"));
    });

    auto left_column = ResizableSplitTop(
        top_left, bottom_left, &top_window_height
    );

    auto layout = ResizableSplitLeft(
        left_column, right_window, &left_column_width
    );

    screen.Loop(layout);
}