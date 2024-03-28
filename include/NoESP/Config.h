#pragma once

#include <RE/Skyrim.h>
#include <SimpleIni.h>

#include <chrono>
#include <filesystem>
#include <functional>
#include <string>

#include "NoESP/Log.h"

namespace NoESP::Config {
    bool LogToConsole                  = false;
    bool SearchObjectReferencesOnStart = true;
    bool LogObjectSearch               = false;

    void LoadFromIni(
        const std::function<void(int searchIndex, double radius, long interval)>& objectSearchConfigEntryCallback
    ) {
        auto iniPath = std::filesystem::current_path() / "Data" / "SKSE" / "Plugins" / "no-esp.ini";
        if (std::filesystem::is_regular_file(iniPath)) {
            try {
                CSimpleIni ini;
                auto       loadError = ini.LoadFile(iniPath.string().c_str());
                if (loadError == SI_OK) {
                    // [Logging] bLogToConsole
                    LogToConsole = ini.GetBoolValue("Logging", "bLogToConsole", false);
                    if (LogToConsole) {
                        RE::ConsoleLog::GetSingleton()->Print("Console logging enabled.");
                    }
                    // [Logging] bLogObjectSearch
                    LogObjectSearch = ini.GetBoolValue("Logging", "bLogObjectSearch", false);
                    // [Bindings] bSearchReferencesOnLoad
                    SearchObjectReferencesOnStart = ini.GetBoolValue("Bindings", "bSearchReferencesOnLoad", false);
                    // [ObjectSearch] fSearch1Radius iSearchIntervalMs
                    int i = 1;
                    while (ini.GetValue("ObjectSearch", std::format("fSearch{}Radius", i).c_str())) {
                        double radius =
                            ini.GetDoubleValue("ObjectSearch", std::format("fSearch{}Radius", i).c_str(), 0);
                        long interval =
                            ini.GetLongValue("ObjectSearch", std::format("iSearch{}IntervalMs", i).c_str(), 0);
                        std::thread t([i, radius, interval, objectSearchConfigEntryCallback]() {
                            objectSearchConfigEntryCallback(i, radius, interval);
                        });
                        t.detach();
                        i++;
                    }
                } else {
                    logger::info("Failed to parse .ini {}", iniPath.string());
                }
            } catch (...) {
                logger::info("Failed to load .ini {}", iniPath.string());
            }
        }
    }
}
