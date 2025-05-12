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

#pragma once

#include <iostream>
#include <chrono>
#include <iomanip>

namespace redsafe::apiserver::util
{
    /**
    *   [YYYY-MM-DD HH:MM:SS:mmm]
    */
    inline std::string current_timestamp()
    {
        using namespace std::chrono;
        auto now = system_clock::now();
        auto ms = duration_cast<milliseconds>(now.time_since_epoch()) % 1000;
        auto in_time_t = system_clock::to_time_t(now);
        std::tm buf;
#if defined(_WIN32) || defined(_WIN64)
    localtime_s(&buf, &in_time_t);
#else
    localtime_r(&in_time_t, &buf);
#endif
        std::ostringstream strTime;
        strTime << '['
                << std::put_time(&buf, "%Y-%m-%d %H:%M:%S")
                << ':' << std::setw(3) << std::setfill('0') << ms.count()
                << "] ";
        return strTime.str();
    }
}