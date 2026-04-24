#pragma once
#include <string>

namespace tdsys::fix {

    class FixEngine {
    public:
        bool send_order(const std::string& req) {
            return true;
        }
    };

}