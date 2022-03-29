#include <format>

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX

#include "RE/Skyrim.h"
#include "SKSE/SKSE.h"

#include <spdlog/sinks/basic_file_sink.h>
#include <xbyak/xbyak.h>
#include <SimpleIni.h>

namespace logger = SKSE::log;
namespace string = SKSE::stl::string;
using namespace std::literals;

#include "ScriptsWithoutESP/System.h"
#include "ScriptsWithoutESP/PapyrusInterface.h"

extern "C" __declspec(dllexport) bool SKSEAPI SKSEPlugin_Load(const SKSE::LoadInterface* skse) {
    SKSE::Init(skse);
    SKSE::GetMessagingInterface()->RegisterListener([](SKSE::MessagingInterface::Message* message){
        if (message->type == SKSE::MessagingInterface::kDataLoaded) {
            ScriptsWithoutESP::System::Start();
            System::ListenForReferences();
            //System::ListenForObjectLoading();
            //System::ListenForFirstLocationLoad();
            //System::ListenForCellLoadEvents();
            SKSE::GetPapyrusInterface()->Register(ScriptsWithoutESP::PapyrusInterface::BIND);
        } else if (message->type == SKSE::MessagingInterface::kNewGame || message->type == SKSE::MessagingInterface::kPostLoadGame) {
            System::GetSingleton().BindFormIdsToScripts();
        }
    });
    return true;
}
//
extern "C" __declspec(dllexport) bool SKSEAPI SKSEPlugin_Query(const SKSE::QueryInterface*, SKSE::PluginInfo* info) {
    info->infoVersion = SKSE::PluginInfo::kVersion;
    info->name = "ScriptsWithoutESP";
    info->version = 1;
    return true;
}

extern "C" __declspec(dllexport) constinit auto SKSEPlugin_Version = [](){
    SKSE::PluginVersionData version;
    version.PluginName("ScriptsWithoutESP");
    version.PluginVersion({ 0, 0, 1 });
    version.CompatibleVersions({ SKSE::RUNTIME_LATEST });
    version.UsesAddressLibrary(true); // Not really necessary or is it?
    return version;
}();
