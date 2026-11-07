#pragma once

#include "robot_env.hpp"

#include "ftxui.hpp"

#include <algorithm>
#include <memory>
#include <string>
#include <vector>

using namespace ftxui;

inline Component SourceWindow(const RobotEnv &env) {
  struct State {
    bool enabled = true;
    int offset = 0;
  };

  std::vector<std::string> lines = env.program_lines;
  auto state = std::make_shared<State>();

  auto toggle = Checkbox("source", &state->enabled);

  auto component = Renderer(toggle, [state, toggle, &lines] {
    constexpr int height = 15;
    const int line_count = static_cast<int>(lines.size());

    state->offset =
        std::clamp(state->offset, 0, std::max(0, line_count - height));

    Elements content;

    if (state->enabled) {
      for (int i = state->offset;
           i < std::min(state->offset + height, line_count); ++i) {
        content.push_back(text(lines[i]));
      }
    } else {
      content.push_back(text("Source disabled") | dim);
    }

    return window(text("source"),
                  vbox({toggle->Render(), separator(),
                        vbox(std::move(content)) | vscroll_indicator | frame |
                            size(HEIGHT, EQUAL, height)}));
  });

  return component | CatchEvent([state](Event event) {
           if (!state->enabled)
             return false;

           if (event == Event::ArrowDown || event == Event::Character('j')) {
             state->offset++;
             return true;
           }

           if (event == Event::ArrowUp || event == Event::Character('k')) {
             state->offset--;
             return true;
           }

           return false;
         });
}
