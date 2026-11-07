#include "robot_env.hpp"
#include <fstream>
#include <iostream>

int main(int argc, char *argv[]) {
  bool quiet = false;
  std::string inputFile;
  std::string outputFile;

  // Parse arguments
  if (argc >= 2) {
    if (std::string(argv[1]) == "-q") {
      quiet = true;
    }
  }

  // Capture input and output filenames
  if (quiet) {
    if (argc < 4) {
      std::cerr << "Usage: " << argv[0] << " -q input.txt output.txt\n";
      return 1;
    }

    inputFile = argv[2];
    outputFile = argv[3];
    std::cout << "input: " << inputFile << " output: " << outputFile
              << std::endl;
    std::ifstream f(inputFile);
    RobotEnv env = RobotEnv(inputFile);
    auto stepOut = env.step();
    bool running = stepOut.log.has_value();
    std::ofstream out(outputFile);
    while (running) {
      out << stepOut.log.value() << '\n';
      std::cout << stepOut.log.value() << '\n';
      stepOut = env.step();
      running = stepOut.log.has_value();

      out.close();
    }
  } else {
    if (argc < 3) {
      std::cerr << "Usage: " << argv[0] << " input.txt output.txt\n";
      return 1;
    }

    inputFile = argv[1];
    outputFile = argv[2];

    // Normal behavior
    std::cout << "Normal mode: the fuckass ui will go here\n";
  }

  return 0;
}
