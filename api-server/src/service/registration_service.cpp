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

#include "registration_service.hpp"
#include "../model/model.hpp"

namespace redsafe::apiserver::service
{
    RegistrationService::RegistrationService(std::string serial_number)
        : serial_number_(serial_number)
    {
    }

    json RegistrationService::registerUser()
    {
        if (std::make_shared<redsafe::apiserver::model::sql::TableCreator>(serial_number_)->CreateTable())
            return json{{"register", "fail"}};
        return json{{"register", "successfully"}};
    }
}