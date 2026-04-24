#pragma once

#include "Shared/SharedContainers.hpp"
#include "TDSystem/TradingSysType.h"
#include "TDSystem/TradingSysUtility.hpp"
#include "spdlog/logger.h"
#include <map>
#include <memory>

/************监视兼职监护进程*************/
namespace tdsys {

    enum class ModuleType{

        MDModule,
        OMModule,

    };

    class TradingSysMonitor{

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
            TradingSysMonitor()=default;
            ~TradingSysMonitor()=default;
            void initialize();
            void run();
            void terminate();
            
    };

}//namespace tdsys
