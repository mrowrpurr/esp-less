#pragma once

#include <SKSE/SKSE.h>

#include "Config.h"

namespace logger = SKSE::log;

namespace NoESP {

    namespace Logging {
        void Initialize() {
            auto path = logger::log_directory();
            if (!path) {
                SKSE::stl::report_and_fail("Failed to find standard logging directory");
                return;
            }

            *path /= "no-esp.log";
            auto       sink  = std::make_shared<spdlog::sinks::basic_file_sink_mt>(path->string(), true);
            const auto level = spdlog::level::info;

            auto log = std::make_shared<spdlog::logger>("global log", std::move(sink));
            log->set_level(level);
            log->flush_on(level);

            spdlog::set_default_logger(std::move(log));
            spdlog::set_pattern("%g(%#): [%^%l%$] %v");
        }
    };
}
