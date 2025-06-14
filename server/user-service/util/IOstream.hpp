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
******************************************************************************/

#ifndef REDSAFE_IOSTREAM_UTIL_HPP
#define REDSAFE_IOSTREAM_UTIL_HPP

#include <iostream>
#include <mutex>

namespace redsafe::server::util {
static std::mutex m_Mutex;

// ThreadSafe cout
// cout() << messafe;
struct cout {
  std::unique_lock<std::mutex> m_Lock;

  cout() : m_Lock(std::unique_lock(m_Mutex)) {}

  explicit cout(const cout&) = delete;
  ~cout() = default;

  template <typename T>
  cout& operator<<(const T& message) {
    std::cout << message;
    return *this;
  }

  cout& operator<<(std::ostream& (*fp)(std::ostream&)) {
    std::cout << fp;
    return *this;
  }
};

// ThreadSafe cerr
// cerr() << messafe;
struct cerr {
  std::unique_lock<std::mutex> m_Lock;

  cerr() : m_Lock(std::unique_lock(m_Mutex)) {}

  explicit cerr(const cerr&) = delete;
  ~cerr() = default;

  template <typename T>
  cerr& operator<<(const T& message) {
    std::cerr << message;
    return *this;
  }

  cerr& operator<<(std::ostream& (*fp)(std::ostream&)) {
    std::cerr << fp;
    return *this;
  }
};
} // namespace redsafe::apiserver::util

#endif