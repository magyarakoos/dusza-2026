#include "ftxui.hpp"

using namespace ftxui;

inline Component SourceWindow() {
  bool enabled = true;

  // Toggle in the top-left corner.
  auto toggle = Checkbox("source", &enabled);

  // Example source content.
  std::vector<std::string> lines;
  for (int i = 1; i <= 100; ++i) {
    lines.push_back("Source line " + std::to_string(i));
  }

  auto content = Renderer([&] {
    Elements elements;
    if (enabled) {
      for (const auto &line : lines) {
        elements.push_back(text(line));
      }
    } else {
      elements.push_back(text("Source disabled") | dim);
    }

    return vbox(std::move(elements)) | vscroll_indicator | frame |
           size(HEIGHT, LESS_THAN, 20);
  });

  // Checkbox + scrollable content inside the window.
  auto component = Container::Vertical({
      toggle,
      content,
  });

  return Renderer(component, [&] {
    return window(text("source"), vbox({
                                      toggle->Render(),
                                      separator(),
                                      content->Render(),
                                  }));
  });
}
