#include <bandit/bandit.h>
#include <snowhouse/snowhouse.h>
#include <spdlog/spdlog.h>
#include <string.h>

#include <format>
#include <iostream>
#include <string_view>

using namespace bandit;
using namespace snowhouse;

namespace NoESP {
    template <class... Types>
    void Log(const std::string text, const Types &...args) {
        std::cout << std::format(text, args...) + "\n";
    };
}

namespace SpecHelper {
    int runBandit(int argc, char *argv[]);
}
