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
#include <thread>

#include "config.hpp"
#include "http/httpserver.hpp"

int main(const int argc, char* argv[]) {
  try {
    const redsafe::server::Server server(SERVER_PORT);
    server.start();
    std::this_thread::sleep_for(std::chrono::hours{24 * 365 * 1});
  } catch (const std::exception& e) {
    std::cerr << "Server error: " << e.what() << "\n";
    std::cin.get();
    return 1;
  }
  return 0;
}