#pragma once
#include "TDSystem/TradingSysType.h"
#include "Shared/SharedContainers.hpp"
#include <map>
#include <memory>

namespace tdsys {

    class TradingSysMonitor {
    private:
        std::shared_ptr<TDSYSLoggerType> logger;
        std::map<std::string, SystemMessageQueue*> moduleMsgQueues;
        SharedContainersManager manager;
        SharedSystemMsgQueues* sharedSystemMsgQueues;
        SystemMessageQueue* moSysMsgQueue;

        void initialSysMsgQueue();
        void MDMService();
        void OMMService();
        void checkModuleRunning();
        void checkMsg();
        void handleTerminate();
        void handleRegister(SystemMessage&);
        void handleUnregister(SystemMessage&);
        void release();

    public:
        TradingSysMonitor() = default;
        ~TradingSysMonitor() = default;

        void initialize();
        void run();
        void terminate();
    };

}
