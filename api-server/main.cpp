/******************************************************************************
   Copyright (C) 2025 by CHEN,BO-EN <chenboen931204@gmail.com>. All Rights Reserved.
  
   This file and its contents are proprietary and confidential.
   Unauthorized reproduction, distribution, or modification is strictly prohibited.
  
   Without the prior written permission of CHEN,BO-EN , you may not:
     1. Modify, adapt, or create derivative works of this source code;
     2. Reverse engineer, decompile, or otherwise attempt to derive the source code;
     3. Distribute, display, or otherwise use this source code or its derivatives in any form.
  
   For licensing inquiries or to obtain a formal license, please contact:
*******************************************************************************/

#include <iostream>

#include "include/api-server.hpp"
#include "config.hpp"

int main(const int argc, char* argv[])
{
    int port = SERVER_PORT;
    for (int i = 1; i < argc; ++i)
        if (std::strcmp(argv[i], "-p") == 0 && i + 1 < argc)
        {
            const char* arg = argv[++i];
            errno = 0;
            char* endptr = nullptr;
            const long val = std::strtol(arg, &endptr, 10);
            if (errno != 0 || endptr == arg || *endptr != '\0' ||
                val < std::numeric_limits<int>::min() ||
                val > std::numeric_limits<int>::max()) {
                std::cerr << "Invalid port number: " << arg << std::endl;
                return 1;
            }
            port = static_cast<int>(val);
        }
    try
    {
        const redsafe::apiserver::Server server(port);
        server.start();
        while (true)
        {
            std::string input;
            // std::cout << "RED-Safe~/$ ";
            std::getline(std::cin, input);
            if (input == "exit")
                return 0;
            if (input == "clear")
                [[maybe_unused]] auto e = system("clear");
        }
    }
    catch(const std::exception& e)
    {
        std::cerr << "Server error: " << e.what() << "\n";
        std::cin.get();
        return 1;
    }
}
