//auto* keyword = form->As<RE::BGSKeyword>();

#pragma once

#include <atomic>
#include <chrono>
#include <regex>
#include <vector>

#include <RE/C/ConsoleLog.h>
#include <RE/T/TESDataHandler.h>
#include <RE/T/TESObjectREFR.h>
#include <REL/Relocation.h>

#include "Log.h"
#include "AutoBindingsFile.h"
#include "OnActorLocationChangeEventSink.h"
#include "PapyrusScriptBindings.h"
#include "Utilities.h"

using namespace std::chrono_literals;

using namespace SKSE::stl;
using namespace REL;
using namespace RE::BSScript;
using namespace RE::BSScript::Internal;

// TODO: split into some lovely organized files <3

// I want this file under like 50 LOC plz.

namespace NoESP {

    // From Base Object Swapper - play around, understand it, rewrite it in my style.
    // https://github.com/powerof3/BaseObjectSwapper/blob/f636b8cc7079ddebd63eb761e99af95b29884ccc/src/PCH.h#L17-L27
    template <class F, class T>
    void vfunc() {
        REL::Relocation<std::uintptr_t> vtable{ F::VTABLE[0] };
        T::func = vtable.write_vfunc(T::size, T::thunk);
    }

    class System {

        std::atomic<bool> _loaded = false;

        std::unordered_map<std::string, bool> _linkedScriptsWorkOK;

        // [Generic Forms]
        // This is taken directly from the AutoBindings and maps to whatever is defined in the file.
        // The collections BELOW are inferred from these raw form bindings.
        std::unordered_map<RE::FormID, std::set<std::string>> _formIdsToScriptNames;

        // [Base Forms]
        // Map reference base form IDs --> scripts to attach on object load
        std::unordered_map<RE::FormID, std::set<std::string>> _baseFormIdsToScriptNames;

        // [Keywords]
        std::unordered_map<RE::BGSKeyword*, std::set<std::string>> _keywordIdsToScriptNames;

        // [Form lists]
        std::unordered_map<RE::BGSListForm*, std::set<std::string>> _formListIdsToScriptNames;

        // [Editor ID matchers]
        std::vector<std::pair<EditorIdMatcher, std::string>> _editorIdMatcherPairs;

        System() = default;

    public:
        System(const System&) = delete;
        System &operator=(const System&) = delete;
        static System& GetSingleton() {
            static System system;
            return system;
        }

        bool IsLoadedOrSetLoaded() { return _loaded.exchange(true); }
        void SetLoaded(bool value = true) { _loaded = value; }

        void SetScriptLinkWorkedOK(const std::string& scriptName, bool value) { _linkedScriptsWorkOK.try_emplace(scriptName, value); }
        bool HasScriptBeenLinked(const std::string& scriptName) { return _linkedScriptsWorkOK.contains(scriptName); }
        bool ScriptLinkWasSuccessful(const std::string& scriptName) { return _linkedScriptsWorkOK[scriptName]; }
        bool TryLinkScript(const std::string& scriptName) {
            if (HasScriptBeenLinked(scriptName)) return ScriptLinkWasSuccessful(scriptName);
            auto* vm = VirtualMachine::GetSingleton();
             try {
                 vm->linker.Process(scriptName);
                 SetScriptLinkWorkedOK(scriptName, true);
                 return true;
             } catch (...) {
                 SetScriptLinkWorkedOK(scriptName, false);
                 return false;
             }
        }

        void AddFormIdForScript(RE::FormID formId, const std::string& scriptName) {
            if (_formIdsToScriptNames.contains(formId)) {
                _formIdsToScriptNames[formId].insert(scriptName);
            } else {
                std::set<std::string> scriptNames{scriptName};
                _formIdsToScriptNames.try_emplace(formId, scriptNames);
            }
        }
        void AddBaseFormIdForScript(RE::FormID formId, const std::string& scriptName) {
            if (_baseFormIdsToScriptNames.contains(formId)) {
                _baseFormIdsToScriptNames[formId].insert(scriptName);
            } else {
                std::set<std::string> scriptNames{scriptName};
                _baseFormIdsToScriptNames.try_emplace(formId, scriptNames);
            }
        }
        void AddKeywordIdForScript(RE::BGSKeyword* keyword, const std::string& scriptName) {
            if (_keywordIdsToScriptNames.contains(keyword)) {
                _keywordIdsToScriptNames[keyword].insert(scriptName);
            } else {
                std::set<std::string> scriptNames{scriptName};
                _keywordIdsToScriptNames.try_emplace(keyword, scriptNames);
            }
        }
        void AddFormListIdForScript(RE::BGSListForm* formList, const std::string& scriptName) {
            if (_formListIdsToScriptNames.contains(formList)) {
                _formListIdsToScriptNames[formList].insert(scriptName);
            } else {
                std::set<std::string> scriptNames{scriptName};
                _formListIdsToScriptNames.try_emplace(formList, scriptNames);
            }
        }
        void AddEditorIdMatcherForScript(const EditorIdMatcher& matcher, const std::string& scriptName) {
            std::pair<EditorIdMatcher, std::string> pair{matcher, scriptName};
            _editorIdMatcherPairs.emplace_back(pair);
        }

        void BindFormIdsToScripts() {
            for (const auto& [formId, scriptNames] : _formIdsToScriptNames) {
                for (const auto& scriptName : scriptNames) {
                    TryLinkScript(scriptName);
                    PapyrusScriptBindings::BindToFormId(scriptName, formId);
                }
            }
        }

        std::set<std::string>& ScriptsForBaseForm(RE::FormID baseFormId) { return _baseFormIdsToScriptNames[baseFormId]; }
        std::unordered_map<RE::BGSKeyword*, std::set<std::string>>& GetScriptNamesForKeywords() { return _keywordIdsToScriptNames; }
        std::unordered_map<RE::BGSListForm*, std::set<std::string>>& GetScriptNamesForFormLists() { return _formListIdsToScriptNames; }
        std::vector<std::pair<EditorIdMatcher, std::string>> GetEditorIdMatcherPairs() { return _editorIdMatcherPairs; }

        // You gurl, move this to a better place!
        static bool DoesEditorIdMatch(const EditorIdMatcher& matcher, const std::string& editorIdText) {
            Log("Does editor ID '{}' match '{}'", editorIdText, matcher.Text);
            if (editorIdText.empty()) return false;
            std::string editorId = Utilities::ToLowerCase(editorIdText);
            switch (matcher.Type) {
                case EditorIdMatcherType::Exact:
                    return editorId == Utilities::ToLowerCase(matcher.Text);
                case EditorIdMatcherType::PrefixMatch:
                    return editorId.starts_with(Utilities::ToLowerCase(matcher.Text));
                case EditorIdMatcherType::SuffixMatch:
                    return editorId.ends_with(Utilities::ToLowerCase(matcher.Text));
                case EditorIdMatcherType::PrefixAndSuffixMatch:
                    return editorId.find(Utilities::ToLowerCase(matcher.Text)) != std::string::npos;
                case EditorIdMatcherType::RegularExpression:
                    return std::regex_match(editorId, matcher.RegularExpression);
                default:
                    return false;
            }
        }

        static void TryBindReference(RE::TESObjectREFR* ref) {
            if (ref->IsDeleted()) {
                Log("Not binding to deleted reference");
                return;
            }

            std::set<std::string> scriptsToBind;
            auto& system = System::GetSingleton();
            auto* baseForm = ref->GetBaseObject();

            // Check 3 things...
            // 1: BaseForm
            for (const auto& scriptName : system.ScriptsForBaseForm(baseForm->formID)) {
                scriptsToBind.insert(scriptName);
            }
            // 2: Keywords
            for (const auto& [keyword, scriptNames] : system.GetScriptNamesForKeywords()) {
                if (ref->HasKeyword(keyword)) {
                    for (const auto& scriptName : scriptNames) {
                        scriptsToBind.insert(scriptName);
                    }
                }
            }
            // 3: FormList presence
            for (const auto& [formList, scriptNames] : system.GetScriptNamesForFormLists()) {
                if (formList->HasForm(ref) || formList->HasForm(baseForm)) {
                    for (const auto& scriptName : scriptNames) {
                        scriptsToBind.insert(scriptName);
                    }
                }
            }

            // Bind the scripts!
            for (const auto& scriptName : scriptsToBind) {
                system.TryLinkScript(scriptName);
                PapyrusScriptBindings::BindToForm(scriptName, ref, true);
            }
        }

        struct OnObjectInitialization {
            static void thunk(RE::TESObjectREFR* ref) {
                TryBindReference(ref);
                func(ref);
            }
            static inline REL::Relocation<decltype(thunk)> func;
            static inline constexpr std::size_t size = 0x13;
        };

        static void ListenForReferences() {
            vfunc<RE::TESObjectREFR, OnObjectInitialization>();
        }

        static void CheckForObjectsToAttachScriptsToFromLiterallyEveryFormInTheGame() {
            const auto& [literallyEveryFormInTheGame, lock] = RE::TESForm::GetAllForms();
            for (auto iterator = literallyEveryFormInTheGame->begin(); iterator != literallyEveryFormInTheGame->end(); iterator++) {
                auto* ref = iterator->second->AsReference();
                if (ref) TryBindReference(ref);
            }
        }

        static void ListenForFirstLocationLoad() {
            auto* scriptEvents = RE::ScriptEventSourceHolder::GetSingleton();
            scriptEvents->AddEventSink<RE::TESActorLocationChangeEvent>(new OnActorLocationChangeEventSink([](const RE::TESActorLocationChangeEvent* event){
                if (event->actor->formID == 20) { // The player reference
                    auto& system = System::GetSingleton();
                    if (! system.IsLoadedOrSetLoaded()) {
                        System::GetSingleton().BindFormIdsToScripts();
                        // Make this something you must turn ON in an .ini off by default:
//                        System::CheckForObjectsToAttachScriptsToFromLiterallyEveryFormInTheGame();
                    }
                }
            }));
        }

        static void ReadAutoBindingsFiles() {
            auto& system = System::GetSingleton();
            AutoBindingsFile::Read([&system](const BindingDefinition& entry){
                RE::TESForm* form = nullptr;
                if (entry.Type == BindingDefinitionType::FormID && entry.Plugin.empty()) {
                    form = RE::TESForm::LookupByID(entry.FormID);
                    if (! form) Log("({}:{}) Form not found: '{:x}'", entry.Filename, entry.ScriptName, entry.FormID);
                } else if (entry.Type == BindingDefinitionType::FormID && ! entry.Plugin.empty()) {
                    form = RE::TESDataHandler::GetSingleton()->LookupForm(entry.FormID, entry.Plugin);
                    if (!form) Log("({}:{}) Form not found from plugin '{}': '{:x}'", entry.Filename, entry.ScriptName, entry.Plugin, entry.FormID);
                } else if (entry.Type == BindingDefinitionType::EditorID) {
                    if (entry.EditorIdMatcher.Type == EditorIdMatcherType::Exact) {
                        form = RE::TESForm::LookupByEditorID(entry.EditorIdMatcher.Text);
                        if (! form) Log("({}:{}) Form not found by editor ID: '{}'", entry.Filename, entry.ScriptName, entry.EditorIdMatcher.Text);
                    } else {
                        Log("Add editor ID matcher '{}' for '{}'", entry.EditorIdMatcher.Text, entry.ScriptName);
                        system.AddEditorIdMatcherForScript(entry.EditorIdMatcher, entry.ScriptName);
                    }
                }
                if (form) {
                    system.AddFormIdForScript(form->formID, entry.ScriptName);
                    if (! form->AsReference()) {
                        if (form->GetFormType() == RE::FormType::Keyword) {
                            system.AddKeywordIdForScript(form->As<RE::BGSKeyword>(), entry.ScriptName);
                        } else if (form->GetFormType() == RE::FormType::FormList) {
                            system.AddFormListIdForScript(form->As<RE::BGSListForm>(), entry.ScriptName);
                        } else {
                            system.AddBaseFormIdForScript(form->formID, entry.ScriptName);
                        }
                    }
                }
            });
        }
    };
}
