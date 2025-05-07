#include "registration_service.hpp"

namespace redsafe::apiserver::service
{
    RegistrationService::RegistrationService(std::string serial_number)
        : serial_number_(serial_number)
    {
    }

    json RegistrationService::registerUser()
    {
        return json{{"register", "successfully"}};
    }
}