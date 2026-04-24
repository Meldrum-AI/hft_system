#pragma once

#include <spdlog/spdlog.h>
#include <spdlog/logger.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/rotating_file_sink.h>

#ifndef NDEBUG
#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_DEBUG
#else
#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_WARN
#endif

// 日志宏定义（和你原生项目完全一致）
#define TDLOG_DEBUG(logger, ...)    SPDLOG_LOGGER_DEBUG(logger, __VA_ARGS__)
#define TDLOG_INFO(logger, ...)     SPDLOG_LOGGER_INFO(logger, __VA_ARGS__)
#define TDLOG_WARN(logger, ...)     SPDLOG_LOGGER_WARN(logger, __VA_ARGS__)
#define TDLOG_ERROR(logger, ...)    SPDLOG_LOGGER_ERROR(logger, __VA_ARGS__)
#define TDLOG_CRITICAL(logger, ...) SPDLOG_LOGGER_CRITICAL(logger, __VA_ARGS__)

namespace tdsys
{
    class LogManager
    {
    public:
        static std::shared_ptr<spdlog::logger> CreateLogger(const std::string& loggerName,
            const std::string& filePath,
            std::size_t maxSize = 1024 * 1024 * 5,
            std::size_t maxCnt = 3)
        {
            try
            {
                auto fileSink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(filePath, maxSize, maxCnt);
                auto consoleSink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();

                std::vector<spdlog::sink_ptr> sinks{ fileSink, consoleSink };
                auto logger = std::make_shared<spdlog::logger>(loggerName, sinks.begin(), sinks.end());

                logger->set_pattern("[%Y-%m-%d %H:%M:%S.%f] [%l] [%n] %v");
                logger->flush_on(spdlog::level::info);
                spdlog::register_logger(logger);

                return logger;
            }
            catch (...)
            {
                return nullptr;
            }
        }
    };
}