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

#ifndef REDSAFE_IOSAPP_SERVICE_HPP
#define REDSAFE_IOSAPP_SERVICE_HPP

#include "../util/response.hpp"

namespace redsafe::server::service::IOSAPP {
class IOSAPPService {
 public:
  static util::Result start(
      const std::string& ios_device_id,
      const std::string& user_id,
      const std::string& apns_token,
      const std::string& device_name);
};
} // namespace redsafe::apiserver::service::IOSAPP

#endif