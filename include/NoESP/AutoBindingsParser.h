#pragma once

#include <RE/F/FormTypes.h>

#include <filesystem>
#include <format>
#include <fstream>
#include <functional>
#include <regex>
#include <sstream>
#include <string>

#include "BindingDefinition.h"
#include "NoESP/Log.h"
#include "Utilities.h"

namespace NoESP::AutoBindingsFile {

    // https://stackoverflow.com/a/40903508
    std::string ReadTextFile(const std::filesystem::path& path) {
        // Open the stream to 'lock' the file.
        std::ifstream f(path, std::ios::in | std::ios::binary);

        // Obtain the size of the file.
        const auto sz = std::filesystem::file_size(path);

        // Create a buffer.
        std::string result(sz, '\0');

        // Read the whole file into the buffer.
        f.read(result.data(), sz);

        return result;
    }

    EditorIdMatcher ParseEditorIdMatchText(const std::string& editorIdText) {
        EditorIdMatcher matcher;
        auto            editorId = Utilities::ToLowerCase(editorIdText);
        if (editorId.starts_with('*') && editorId.ends_with('*') && editorId.length() > 2) {
            matcher.Type = EditorIdMatcherType::PrefixAndSuffixMatch;
            matcher.Text = editorId.substr(1, editorId.length() - 2);
        } else if (editorId.starts_with('*')) {
            matcher.Type = EditorIdMatcherType::SuffixMatch;
            matcher.Text = editorId.substr(1);
        } else if (editorId.ends_with('*')) {
            matcher.Type = EditorIdMatcherType::PrefixMatch;
            matcher.Text = editorId.substr(0, editorId.length() - 1);
        } else if (editorId.starts_with('/') && editorId.ends_with('/') && editorId.length() > 2) {
            matcher.Type               = EditorIdMatcherType::RegularExpression;
            auto regularExpressionText = editorId.substr(1, editorId.length() - 2);
            matcher.Text               = regularExpressionText;
            matcher.RegularExpression  = std::regex(regularExpressionText, std::regex_constants::icase);
        } else {
            matcher.Type = EditorIdMatcherType::Exact;
            matcher.Text = editorId;
        }
        return matcher;
    }

    // Recurse me!
    void ParsePropertiesFromLine(FormPropertyMap& properties, std::string& line) {
        // from the right side of the line, look for: A="B C" or D=1 or E=true
        // etc
        static auto rightHandSideQuotedPropertyAssignment =
            std::regex("\\s+([^=\\s]+)=\"([^\"]+)\"$", std::regex_constants::icase);
        static auto rightHandSidePropertyAssignment =
            std::regex(R"(\s+([^=\s]+)=([^=]+)$)", std::regex_constants::icase);
        static auto slashNregex = std::regex("\\\\n");
        static auto slashTregex = std::regex("\\\\t");
        try {
            std::smatch matches;
            if (std::regex_search(line, matches, rightHandSideQuotedPropertyAssignment)) {
                PropertyValue property;
                property.PropertyName      = Utilities::ToLowerCase(matches[1].str());
                property.PropertyValueText = std::regex_replace(matches[2].str(), slashNregex, "\n");
                property.PropertyValueText = std::regex_replace(property.PropertyValueText, slashTregex, "\t");
                properties.insert_or_assign(property.PropertyName, property);

                line = line.substr(0, matches.position());
                ParsePropertiesFromLine(properties, line);
            } else if (std::regex_search(line, matches, rightHandSidePropertyAssignment)) {
                PropertyValue property;
                property.PropertyName      = Utilities::ToLowerCase(matches[1].str());
                property.PropertyValueText = std::regex_replace(matches[2].str(), slashNregex, "\n");
                property.PropertyValueText = std::regex_replace(property.PropertyValueText, slashTregex, "\t");
                properties.insert_or_assign(property.PropertyName, property);

                line = line.substr(0, matches.position());
                ParsePropertiesFromLine(properties, line);
            }
        } catch (...) {
            logger::info("Error parsing properties from line '{}'", line);
        }
    }

    std::unordered_map<std::string, RE::FormType> FORM_TYPES_BY_NAME{
        {{"none", RE::FormType::None},
         {"tes4", RE::FormType::PluginInfo},
         {"plugininfo", RE::FormType::PluginInfo},
         {"grup", RE::FormType::FormGroup},
         {"formgroup", RE::FormType::FormGroup},
         {"gmst", RE::FormType::GameSetting},
         {"gamesetting", RE::FormType::GameSetting},
         {"kywd", RE::FormType::Keyword},
         {"keyword", RE::FormType::Keyword},
         {"lcrt", RE::FormType::LocationRefType},
         {"locationreftype", RE::FormType::LocationRefType},
         {"aact", RE::FormType::Action},
         {"action", RE::FormType::Action},
         {"txst", RE::FormType::TextureSet},
         {"textureset", RE::FormType::TextureSet},
         {"micn", RE::FormType::MenuIcon},
         {"menuicon", RE::FormType::MenuIcon},
         {"glob", RE::FormType::Global},
         {"global", RE::FormType::Global},
         {"clas", RE::FormType::Class},
         {"class", RE::FormType::Class},
         {"fact", RE::FormType::Faction},
         {"faction", RE::FormType::Faction},
         {"hdpt", RE::FormType::HeadPart},
         {"headpart", RE::FormType::HeadPart},
         {"eyes", RE::FormType::Eyes},
         {"eyes", RE::FormType::Eyes},
         {"race", RE::FormType::Race},
         {"race", RE::FormType::Race},
         {"soun", RE::FormType::Sound},
         {"sound", RE::FormType::Sound},
         {"aspc", RE::FormType::AcousticSpace},
         {"acousticspace", RE::FormType::AcousticSpace},
         {"skil", RE::FormType::Skill},
         {"skill", RE::FormType::Skill},
         {"mgef", RE::FormType::MagicEffect},
         {"magiceffect", RE::FormType::MagicEffect},
         {"scpt", RE::FormType::Script},
         {"script", RE::FormType::Script},
         {"ltex", RE::FormType::LandTexture},
         {"landtexture", RE::FormType::LandTexture},
         {"ench", RE::FormType::Enchantment},
         {"enchantment", RE::FormType::Enchantment},
         {"spel", RE::FormType::Spell},
         {"spell", RE::FormType::Spell},
         {"scrl", RE::FormType::Scroll},
         {"scroll", RE::FormType::Scroll},
         {"acti", RE::FormType::Activator},
         {"activator", RE::FormType::Activator},
         {"tact", RE::FormType::TalkingActivator},
         {"talkingactivator", RE::FormType::TalkingActivator},
         {"armo", RE::FormType::Armor},
         {"armor", RE::FormType::Armor},
         {"book", RE::FormType::Book},
         {"cont", RE::FormType::Container},
         {"container", RE::FormType::Container},
         {"door", RE::FormType::Door},
         {"ingr", RE::FormType::Ingredient},
         {"ingredient", RE::FormType::Ingredient},
         {"ligh", RE::FormType::Light},
         {"light", RE::FormType::Light},
         {"misc", RE::FormType::Misc},
         {"appa", RE::FormType::Apparatus},
         {"apparatus", RE::FormType::Apparatus},
         {"stat", RE::FormType::Static},
         {"static", RE::FormType::Static},
         {"scol", RE::FormType::StaticCollection},
         {"staticcollection", RE::FormType::StaticCollection},
         {"mstt", RE::FormType::MovableStatic},
         {"movablestatic", RE::FormType::MovableStatic},
         {"gras", RE::FormType::Grass},
         {"grass", RE::FormType::Grass},
         {"tree", RE::FormType::Tree},
         {"flor", RE::FormType::Flora},
         {"flora", RE::FormType::Flora},
         {"furn", RE::FormType::Furniture},
         {"furniture", RE::FormType::Furniture},
         {"weap", RE::FormType::Weapon},
         {"weapon", RE::FormType::Weapon},
         {"weapons", RE::FormType::Weapon},
         {"ammo", RE::FormType::Ammo},
         {"npc_", RE::FormType::NPC},
         {"npc", RE::FormType::NPC},
         {"lvln", RE::FormType::LeveledNPC},
         {"levelednpc", RE::FormType::LeveledNPC},
         {"keym", RE::FormType::KeyMaster},
         {"keymaster", RE::FormType::KeyMaster},
         {"alch", RE::FormType::AlchemyItem},
         {"alchemy", RE::FormType::AlchemyItem},
         {"alchemyitem", RE::FormType::AlchemyItem},
         {"idlm", RE::FormType::IdleMarker},
         {"idlemarker", RE::FormType::IdleMarker},
         {"note", RE::FormType::Note},
         {"note", RE::FormType::Note},
         {"cobj", RE::FormType::ConstructibleObject},
         {"constructibleobject", RE::FormType::ConstructibleObject},
         {"proj", RE::FormType::Projectile},
         {"projectile", RE::FormType::Projectile},
         {"hazd", RE::FormType::Hazard},
         {"hazard", RE::FormType::Hazard},
         {"slgm", RE::FormType::SoulGem},
         {"soulgem", RE::FormType::SoulGem},
         {"lvli", RE::FormType::LeveledItem},
         {"leveleditem", RE::FormType::LeveledItem},
         {"wthr", RE::FormType::Weather},
         {"weather", RE::FormType::Weather},
         {"clmt", RE::FormType::Climate},
         {"climate", RE::FormType::Climate},
         {"spgdt", RE::FormType::ShaderParticleGeometryData},
         {"shaderparticlegeometrydata", RE::FormType::ShaderParticleGeometryData},
         {"shaderparticlegeometrydatrydata", RE::FormType::ShaderParticleGeometryData},
         {"rfct", RE::FormType::ReferenceEffect},
         {"referenceeffect", RE::FormType::ReferenceEffect},
         {"regn", RE::FormType::Region},
         {"region", RE::FormType::Region},
         {"navi", RE::FormType::Navigation},
         {"navigation", RE::FormType::Navigation},
         {"cell", RE::FormType::Cell},
         {"refr", RE::FormType::Reference},
         {"reference", RE::FormType::Reference},
         {"achra", RE::FormType::ActorCharacter},
         {"actorcharacter", RE::FormType::ActorCharacter},
         {"pmis", RE::FormType::ProjectileMissile},
         {"projectilemissile", RE::FormType::ProjectileMissile},
         {"parw", RE::FormType::ProjectileArrow},
         {"projectilearrow", RE::FormType::ProjectileArrow},
         {"pgre", RE::FormType::ProjectileGrenade},
         {"projectilegrenade", RE::FormType::ProjectileGrenade},
         {"pbea", RE::FormType::ProjectileBeam},
         {"projectilebeam", RE::FormType::ProjectileBeam},
         {"pfla", RE::FormType::ProjectileFlame},
         {"projectileflame", RE::FormType::ProjectileFlame},
         {"pcon", RE::FormType::ProjectileCone},
         {"projectilecone", RE::FormType::ProjectileCone},
         {"pbar", RE::FormType::ProjectileBarrier},
         {"projectilebarrier", RE::FormType::ProjectileBarrier},
         {"phzd", RE::FormType::PlacedHazard},
         {"placedhazard", RE::FormType::PlacedHazard},
         {"wrld", RE::FormType::WorldSpace},
         {"worldspace", RE::FormType::WorldSpace},
         {"land", RE::FormType::Land},
         {"navm", RE::FormType::NavMesh},
         {"navmesh", RE::FormType::NavMesh},
         {"tlod", RE::FormType::TLOD},
         {"dial", RE::FormType::Dialogue},
         {"dialogue", RE::FormType::Dialogue},
         {"info", RE::FormType::Info},
         {"qust", RE::FormType::Quest},
         {"quest", RE::FormType::Quest},
         {"idle", RE::FormType::Idle},
         {"packa", RE::FormType::Package},
         {"package", RE::FormType::Package},
         {"csty", RE::FormType::CombatStyle},
         {"combatstyle", RE::FormType::CombatStyle},
         {"lscr", RE::FormType::LoadScreen},
         {"loadscreen", RE::FormType::LoadScreen},
         {"lvsp", RE::FormType::LeveledSpell},
         {"leveledspell", RE::FormType::LeveledSpell},
         {"anio", RE::FormType::AnimatedObject},
         {"animatedobject", RE::FormType::AnimatedObject},
         {"watr", RE::FormType::Water},
         {"water", RE::FormType::Water},
         {"efsh", RE::FormType::EffectShader},
         {"effectshader", RE::FormType::EffectShader},
         {"toft", RE::FormType::TOFT},
         {"toft", RE::FormType::TOFT},
         {"expl", RE::FormType::Explosion},
         {"explosion", RE::FormType::Explosion},
         {"debr", RE::FormType::Debris},
         {"debris", RE::FormType::Debris},
         {"imgs", RE::FormType::ImageSpace},
         {"imagespace", RE::FormType::ImageSpace},
         {"imad", RE::FormType::ImageAdapter},
         {"imageadapter", RE::FormType::ImageAdapter},
         {"flst", RE::FormType::FormList},
         {"formlist", RE::FormType::FormList},
         {"perk", RE::FormType::Perk},
         {"bptd", RE::FormType::BodyPartData},
         {"bodypartdata", RE::FormType::BodyPartData},
         {"addn", RE::FormType::AddonNode},
         {"addonnode", RE::FormType::AddonNode},
         {"avif", RE::FormType::ActorValueInfo},
         {"actorvalueinfo", RE::FormType::ActorValueInfo},
         {"cams", RE::FormType::CameraShot},
         {"camerashot", RE::FormType::CameraShot},
         {"cpth", RE::FormType::CameraPath},
         {"camerapath", RE::FormType::CameraPath},
         {"vtyp", RE::FormType::VoiceType},
         {"voicetype", RE::FormType::VoiceType},
         {"matt", RE::FormType::MaterialType},
         {"materialtype", RE::FormType::MaterialType},
         {"ipct", RE::FormType::Impact},
         {"impact", RE::FormType::Impact},
         {"ipds", RE::FormType::ImpactDataSet},
         {"impactdataset", RE::FormType::ImpactDataSet},
         {"arma", RE::FormType::Armature},
         {"armature", RE::FormType::Armature},
         {"eczn", RE::FormType::EncounterZone},
         {"encounterzone", RE::FormType::EncounterZone},
         {"lctn", RE::FormType::Location},
         {"location", RE::FormType::Location},
         {"mesg", RE::FormType::Message},
         {"message", RE::FormType::Message},
         {"rgdl", RE::FormType::Ragdoll},
         {"ragdoll", RE::FormType::Ragdoll},
         {"dobjr", RE::FormType::DefaultObject},
         {"defaultobject", RE::FormType::DefaultObject},
         {"lgtm", RE::FormType::LightingMaster},
         {"lightingmaster", RE::FormType::LightingMaster},
         {"musc", RE::FormType::MusicType},
         {"musictype", RE::FormType::MusicType},
         {"fstp", RE::FormType::Footstep},
         {"footstep", RE::FormType::Footstep},
         {"fsts", RE::FormType::FootstepSet},
         {"footstepset", RE::FormType::FootstepSet},
         {"smbno", RE::FormType::StoryManagerBranchNode},
         {"storymanagerbranchnode", RE::FormType::StoryManagerBranchNode},
         {"smqnd", RE::FormType::StoryManagerQuestNode},
         {"storymanagerquestnode", RE::FormType::StoryManagerQuestNode},
         {"smend", RE::FormType::StoryManagerEventNode},
         {"storymanagereventnode", RE::FormType::StoryManagerEventNode},
         {"dlbr", RE::FormType::DialogueBranch},
         {"dialoguebranch", RE::FormType::DialogueBranch},
         {"muste", RE::FormType::MusicTrack},
         {"musictrack", RE::FormType::MusicTrack},
         {"dlvw", RE::FormType::DialogueView},
         {"dialogueview", RE::FormType::DialogueView},
         {"woop", RE::FormType::WordOfPower},
         {"wordofpower", RE::FormType::WordOfPower},
         {"shou", RE::FormType::Shout},
         {"shout", RE::FormType::Shout},
         {"equp", RE::FormType::EquipSlot},
         {"equipslot", RE::FormType::EquipSlot},
         {"rela", RE::FormType::Relationship},
         {"relationship", RE::FormType::Relationship},
         {"scen", RE::FormType::Scene},
         {"scene", RE::FormType::Scene},
         {"astp", RE::FormType::AssociationType},
         {"associationtype", RE::FormType::AssociationType},
         {"otft", RE::FormType::Outfit},
         {"outfit", RE::FormType::Outfit},
         {"arto", RE::FormType::ArtObject},
         {"artobject", RE::FormType::ArtObject},
         {"mato", RE::FormType::MaterialObject},
         {"materialobject", RE::FormType::MaterialObject},
         {"movt", RE::FormType::MovementType},
         {"movementtype", RE::FormType::MovementType},
         {"sndr", RE::FormType::SoundRecord},
         {"soundrecord", RE::FormType::SoundRecord},
         {"dual", RE::FormType::DualCastData},
         {"dualcastdata", RE::FormType::DualCastData},
         {"snct", RE::FormType::SoundCategory},
         {"soundcategory", RE::FormType::SoundCategory},
         {"sopm", RE::FormType::SoundOutputModel},
         {"soundoutputmodel", RE::FormType::SoundOutputModel},
         {"coll", RE::FormType::CollisionLayer},
         {"collisionlayer", RE::FormType::CollisionLayer},
         {"clfm", RE::FormType::ColorForm},
         {"colorform", RE::FormType::ColorForm},
         {"revb", RE::FormType::ReverbParam},
         {"reverbparam", RE::FormType::ReverbParam},
         {"lens", RE::FormType::LensFlare},
         {"lensflare", RE::FormType::LensFlare},
         {"lspr", RE::FormType::LensSprite},
         {"lenssprite", RE::FormType::LensSprite},
         {"voli", RE::FormType::VolumetricLighting},
         {"volumetriclighting", RE::FormType::VolumetricLighting}}
    };

    void ParseFormTypesFromLine(BindingDefinition& def, std::string& line) {
        static auto formTypesPattern          = std::regex(R"(^\s*([^\s]+)\s+\[([^\]]+)\])");
        static auto justTheTypesInTheBrackets = std::regex(R"(\s+\[([^\]]+)\]\s*)");

        std::smatch matches;
        if (std::regex_search(line, matches, formTypesPattern) &&
            std::regex_search(line, matches, justTheTypesInTheBrackets)) {
            auto formTypeNames = Utilities::ToLowerCase(matches[1].str());
            line = line.substr(0, matches.position()) + " " + line.substr(matches.position() + matches.length()) + " ";
            auto index = formTypeNames.find('|');
            while (index != std::string::npos) {
                auto formTypeName = formTypeNames.substr(0, index);
                if (FORM_TYPES_BY_NAME.contains(formTypeName)) {
                    def.FormTypes.insert(FORM_TYPES_BY_NAME[formTypeName]);
                } else {
                    logger::info("Form type not found for name '{}'", formTypeName);
                }
                formTypeNames = formTypeNames.substr(index + 1);
                index         = formTypeNames.find('|');
            }
            if (!formTypeNames.empty()) {
                if (FORM_TYPES_BY_NAME.contains(formTypeNames)) {
                    def.FormTypes.insert(FORM_TYPES_BY_NAME[formTypeNames]);
                } else {
                    logger::info("Form type not found for name '{}'", formTypeNames);
                }
            }
        }
    }

    BindingDefinition ParseLine(std::string line) {
        FormPropertyMap properties;
        ParsePropertiesFromLine(properties, line);

        BindingDefinition entry;
        entry.PropertyValues = properties;

        ParseFormTypesFromLine(entry, line);

        static auto scriptNameWithPluginFormID = std::regex(
            R"(^\s*([^\s]+)\s+0x([^\s]+)\s+([^\s]+)\s*$)",
            std::regex_constants::icase
        );  // TODO - remove icase
        static auto scriptNameWithSkyrimFormID =
            std::regex(R"(^\s*([^\s]+)\s+0x([^\s]+)\s*$)", std::regex_constants::icase);
        static auto scriptNameWithEditorID = std::regex(R"(^\s*([^\s]+)\s+([^\s]+)\s*$)", std::regex_constants::icase);
        static auto scriptNameOnly         = std::regex(R"(^\s*([^\s]+)\s*$)", std::regex_constants::icase);
        static auto scriptName             = std::regex(R"(^\s*([^\s]+)\s*)", std::regex_constants::icase);
        std::smatch matches;
        try {
            if (std::regex_search(line, matches, scriptNameWithPluginFormID)) {
                entry.Type       = BindingDefinitionType::FormID;
                entry.ScriptName = matches[1].str();
                auto formIdHex   = matches[2].str();
                try {
                    entry.FormID = std::stoi("0x" + formIdHex, nullptr, 16);
                } catch (...) {
                    logger::info("Invalid FormID '0x{}'", formIdHex);
                }
                entry.Plugin = matches[3].str();
            } else if (std::regex_search(line, matches, scriptNameWithSkyrimFormID)) {
                entry.Type       = BindingDefinitionType::FormID;
                entry.ScriptName = matches[1].str();
                auto formIdHex   = matches[2].str();
                entry.FormID     = std::stoi(formIdHex, nullptr, 16);
            } else if (std::regex_search(line, matches, scriptNameWithEditorID)) {
                entry.Type            = BindingDefinitionType::EditorID;
                entry.ScriptName      = matches[1].str();
                entry.EditorIdMatcher = ParseEditorIdMatchText(matches[2].str());
            } else if (!entry.FormTypes.empty() && std::regex_search(line, matches, scriptName)) {
                entry.Type       = BindingDefinitionType::FormType;
                entry.ScriptName = matches[1].str();
            } else if (std::regex_search(line, matches, scriptNameOnly)) {
                entry.Type       = BindingDefinitionType::FormID;
                entry.ScriptName = matches[1].str();
                entry.FormID     = 20;  // 0x14 which is the PlayerRef
            } else {
                logger::info("Unknown/Unsupported Entry Line Syntax: '{}'", line);
            }
        } catch (...) {
            logger::info("Error parsing line: '{}'", line);
        }
        return entry;
    }

    void Read(
        std::function<void(BindingDefinition& entry)> entryCallback,
        const std::string&                            bindingFilesDirectory = "Data/Scripts/AutoBindings"
    ) {
        if (!std::filesystem::is_directory(bindingFilesDirectory)) return;

        for (auto& file : std::filesystem::directory_iterator(bindingFilesDirectory)) {
            if (file.is_regular_file()) {
                try {
                    if (!file.path().string().ends_with(".txt")) continue;
                    auto               text = ReadTextFile(file.path());
                    std::istringstream stringStream(text);
                    for (std::string line; std::getline(stringStream, line);) {
                        if (!line.empty()) {
                            // Strip trailing \r from newline
                            if (line.ends_with("\r")) {
                                line.erase(line.length() - 1);
                            }
                            // Strip anything before comment
                            size_t commentStartIndex = line.find('#');
                            if (commentStartIndex != std::string::npos) {
                                if (commentStartIndex == 0) line = "";
                                else line = line.substr(0, commentStartIndex);
                            }
                            commentStartIndex = line.find(';');
                            if (commentStartIndex != std::string::npos) {
                                if (commentStartIndex == 0) line = "";
                                else line = line.substr(0, commentStartIndex);
                            }
                            commentStartIndex = line.find("//");
                            if (commentStartIndex != std::string::npos) {
                                if (commentStartIndex == 0) line = "";
                                else line = line.substr(0, commentStartIndex);
                            }

                            if (!line.empty()) {
                                // Parse the (trimmed) line
                                try {
                                    auto entry = ParseLine(line);
                                    if (entry.Type != BindingDefinitionType::Invalid) {
                                        entry.Filename = file.path().string();
                                        try {
                                            entryCallback(entry);
                                        } catch (...) {
                                            logger::info(
                                                "[Internal] AutoBinding entry "
                                                "discovery callback failed for "
                                                "'{}'",
                                                line
                                            );
                                        }
                                    }
                                } catch (...) {
                                    logger::info("Failed to parse entry line '{}'", line);
                                }
                            }
                        }
                    }
                } catch (...) {
                    logger::info("Failed to read file '{}'", file.path().string());
                }
            }
        }
    }
}
