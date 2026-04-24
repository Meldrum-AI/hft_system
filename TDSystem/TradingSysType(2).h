#pragma once

#include <chrono>
#include <cstdint>
#include <string>
#include "TradingSysDefine.h"

//spdlog 特殊处理
#ifndef NDEBUG
    #define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_DEBUG
#else
    #define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_WARN
#endif
#include "spdlog/logger.h"
#include "spdlog/spdlog.h"
//宏日志封装，方便按等级消除日志代码
#define TDLOG_DEBUG(logger,...) SPDLOG_LOGGER_DEBUG(logger, ##__VA_ARGS__)
#define TDLOG_INFO(logger,...) SPDLOG_LOGGER_INFO(logger, ##__VA_ARGS__)
#define TDLOG_WARN(logger,...) SPDLOG_LOGGER_WARN(logger, ##__VA_ARGS__)
#define TDLOG_ERROR(logger,...) SPDLOG_LOGGER_ERROR(logger, ##__VA_ARGS__)
#define TDLOG_CRITICAL(logger,...) SPDLOG_LOGGER_CRITICAL(logger, ##__VA_ARGS__)
//非热链路不使用宏日志，通过普通调用方法实现运行时的按配置恢复详细日志以方便debug

namespace tdsys {
    
    //规定日志器类型，方便更换
    using TDSYSLoggerType=spdlog::logger;
    using TimeType=std::chrono::system_clock::time_point;
    
    //enumeration
    enum class SystemMessageType{

        REGISTER,
        UNREGISTER,
        READYSIG,
        PAUSESIG,
        TERMINATESIG,

    };

    //TimeStamp Struct unused
    struct TimeStamp{

        uint64_t timeStamp;
        // 转换为纳秒
        uint64_t nanoseconds() const;
        // 转换为微秒
        uint64_t microseconds() const;
        // 转换为毫秒
        uint64_t milliseconds() const;
        // 转换为秒
        uint64_t seconds() const ;
        //赋值函数
        TimeStamp& operator=(const TimeStamp& other);

    };
    
    //TradingSys InnerCode loader unused
    class InnerCodeHelper{

        public:
            static int getInnerCode(std::string);
            static int innerCodeToOffset(int);
            
    };

    //TradingSys Systematic Message Struct
    struct SystemMessage{
        
        SystemMessageType type;
        TimeType timeStamp;
        char moduleName [TDSYS_MODULE_NAME_LENGTH];
        char senderQueueCode [TDSYS_QUEUE_CODE_LENGTH];
        
    };

    //取时函数，后续可更换实现
    inline 
    auto GetTime(){

        return std::chrono::high_resolution_clock::now();
    
    }

}//namespace tdsys 