/******************************************************************************
   Copyright (C) 2025 by CHEN,BO-EN <chenboen931204@gmail.com>. All Rights
Reserved.

   This file and its contents are proprietary and confidential.
   Unauthorized reproduction, distribution, or modification is strictly
prohibited.

   Without the prior written permission of CHEN,BO-EN , you may not:
     1. Modify, adapt, or create derivative works of this source code;
     2. Reverse engineer, decompile, or otherwise attempt to derive the source
code;
     3. Distribute, display, or otherwise use this source code or its
derivatives in any form.

   For licensing inquiries or to obtain a formal license, please contact:
*******************************************************************************/

#include <iostream>

#include "config.hpp"
#include "include/api-server.hpp"

int main(const int argc, char* argv[]) {
  try {
    const redsafe::apiserver::Server server(SERVER_PORT);
    server.start();
    while (true) {
      std::string input;
      // std::cout << "RED-Safe~/$ ";
      std::getline(std::cin, input);
      if (input == "exit") return 0;
      if (input == "clear") [[maybe_unused]]
        auto e = system("clear");
    }
  } catch (const std::exception& e) {
    std::cerr << "Server error: " << e.what() << "\n";
    std::cin.get();
    return 1;
  }
}
