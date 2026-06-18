// This file is probably in the wrong place; I'll figure it out later.

#include <pch.h>
#include <imgui.h>
#include <game/asset.h>
#include "core/render/uistate.h"
#include <game/rtech/cpakfile.h>
#include <game/rtech/assets/settings.h>
#include <core/render/dx.h>
#include <game/rtech/assets/localization.h>
#include <game/rtech/assets/odl_asset.h>
#include <imgui_internal.h>
#include <misc/imgui_utility.h>

extern CDXParentHandler* g_dxHandler;

const std::unordered_map<char, uint32_t> qualityInts = {
    {'C', 0u}, // COMMON
    {'R', 1u}, // RARE
    {'E', 2u}, // EPIC
    {'L', 3u}, // LEGENDARY
    {'M', 4u}, // MYTHIC
    {'I', 5u}, // ICONIC
};

void ItemflavWindow_GetCharacterDataFromSettings(CAsset* asset)
{
    CUIState& uiState = g_dxHandler->GetUIState();

    CUI_ItemflavCharacter* character = nullptr;

    int characterIdx = 0;

    for (characterIdx = 0; characterIdx < uiState.itemflavData.numCharacters; ++characterIdx)
    {
        if (uiState.itemflavData.characterData[characterIdx].settingsAssetGuid == asset->GetAssetGUID())
        {
            character = &uiState.itemflavData.characterData[characterIdx];
            break;
        }
    }

    assertm(character, "uh oh");

    character->settingsAsset = asset;

    SettingsAsset* const settingsAsset = reinterpret_cast<SettingsAsset*>(reinterpret_cast<CPakAsset*>(asset)->extraData());

    if (settingsAsset->_numFields == 0)
        settingsAsset->ParseSettingsData();
    
    SettingsKVValue_t* value = nullptr;

    if (settingsAsset->GetSettingValue("localizationKey_NAME", &value))
        character->characterName = value->getValue<const char*>();

    if(settingsAsset->GetSettingValue("localizationKey_DESCRIPTION_SHORT", &value))
        character->characterDesc = value->getValue<const char*>();

    if (settingsAsset->GetSettingValue("shippingStatus", &value))
        character->shippingStatus = value->getValue<const char*>();

    if (settingsAsset->GetSettingValue("skins", &value))
    {
        assert(value->type == eSettingsFieldType::ST_ARRAY);

        character->skins.resize(value->numChildren);

        size_t j = 0;
        for (auto& skin : character->skins)
        {
            SettingsKVValue_t* const skinObj = &value->getValue<SettingsKVValue_t*>()[j];

            // first field in the object is always the settings asset path
            skin.assetPath = skinObj->getValue<SettingsKVField_t*>()[0].getValue<const char*>();
            skin.settingsAsset = g_assetData.FindAssetByGUID(RTech::StringToGuid(skin.assetPath));

            SettingsAsset* skinAsset = reinterpret_cast<SettingsAsset*>(reinterpret_cast<CPakAsset*>(skin.settingsAsset)->extraData());

            j++; // increment before we could possibly continue

            // _shared_favorites
            if (skinAsset->uniqueId == 0x053a2537)
                continue;

            if (skinAsset->_numFields == 0)
                skinAsset->ParseSettingsData();

            SettingsKVValue_t* skinNameValue = nullptr;
            assertm(skinAsset->GetSettingValue("localizationKey_NAME", &skinNameValue), "Failed to get skin name");

            SettingsKVValue_t* qualityValue = nullptr;
            assertm(skinAsset->GetSettingValue("quality", &qualityValue), "Failed to get skin quality");

            SettingsKVValue_t* armsModelValue = nullptr;
            assertm(skinAsset->GetSettingValue("armsModel", &armsModelValue), "Failed to get arms model");

            SettingsKVValue_t* bodyModelValue = nullptr;
            assertm(skinAsset->GetSettingValue("bodyModel", &bodyModelValue), "Failed to get body model");

            SettingsKVValue_t* armsModelOdlValue = nullptr;
            assertm(skinAsset->GetSettingValue("armsModel_odlBlob", &armsModelOdlValue), "Failed to get arms model odlBlob");

            SettingsKVValue_t* bodyModelOdlValue = nullptr;
            assertm(skinAsset->GetSettingValue("bodyModel_odlBlob", &bodyModelOdlValue), "Failed to get body model odlBlob");

            skin.localizationKey_NAME = skinNameValue->getValue<const char*>();
            skin.quality = qualityValue->getValue<const char*>();
            skin.qualityIndex = qualityInts.at(skin.quality[0]);
            skin.armsModel = armsModelValue->getValue<const char*>();
            skin.bodyModel = bodyModelValue->getValue<const char*>();

            CPakAsset* armsModelOdlAsset = reinterpret_cast<CPakAsset*>(g_assetData.FindAssetByGUID(RTech::StringToGuid(armsModelOdlValue->getValue<const char*>())));
            CPakAsset* bodyModelOdlAsset = reinterpret_cast<CPakAsset*>(g_assetData.FindAssetByGUID(RTech::StringToGuid(bodyModelOdlValue->getValue<const char*>())));

            if (armsModelOdlAsset)
                skin.armsModelPak = armsModelOdlAsset->extraData<ODLAsset*>()->GetPakName();
            if (bodyModelOdlAsset)
                skin.bodyModelPak = bodyModelOdlAsset->extraData<ODLAsset*>()->GetPakName();
        }

        std::sort(character->skins.begin(), character->skins.end(),
            [](const CUI_ItemflavCharacterSkin& a, const CUI_ItemflavCharacterSkin& b)
            {
                if (a.armsModelPak < b.armsModelPak) return true;
                if (b.armsModelPak < a.armsModelPak) return false;

                return a.qualityIndex < b.qualityIndex;
            }
        );
    }
}

void ItemflavWindow_LocalizationPostLoadCallback(CAsset* asset)
{
    CUIState& uiState = g_dxHandler->GetUIState();

    uiState.itemflavData.localizationAsset = asset;
}

std::string Localize(const char* key)
{
    const char* origKey = key;

    if (strnlen(origKey, 1024) <= 1)
        return origKey;

    // skip over the 
    if (key[0] == '#')
        key++;

    CUIState& uiState = g_dxHandler->GetUIState();

    if (uiState.itemflavData.localizationAsset)
    {
        const LocalizationAsset* loc = reinterpret_cast<CPakAsset*>(uiState.itemflavData.localizationAsset)->extraData<LocalizationAsset*>();
        
        if (auto it = loc->entryMap.find(RTech::StringToGuid(key)); it != loc->entryMap.end())
        {
                return it->second;
        }
    }

    return origKey;
}

void ItemflavWindow_RefreshData(CUIState* uiState)
{
    CUI_ItemflavWindowData* const flavData = &uiState->itemflavData;

    uiState->itemFlavorListAsset = g_assetData.FindAssetByGUID(RTech::StringToGuid("settings\\base_itemflavors.rpak"));

    // If the list asset has now been found, populate the character list
    if (uiState->itemFlavorListAsset)
    {
        const uint64_t localizationGuid = RTech::StringToGuid("localization\\localization_english.rpak");
        if (CAsset* localizationAsset = g_assetData.FindAssetByGUID(localizationGuid))
            ItemflavWindow_LocalizationPostLoadCallback(localizationAsset);
        else
        {
            // Full pak path for the localization_english.rpak file in the same directory as common.rpak
            const std::filesystem::path fullPakPath = reinterpret_cast<CPakFile*>(reinterpret_cast<CPakAsset*>(uiState->itemFlavorListAsset)->GetContainerFile())->GetFilePath().parent_path() / "localization_english.rpak";

            extern void HandlePakLoad(std::vector<std::string> filePaths);

            HandlePakLoad({ fullPakPath.string() });

            g_assetData.AddAssetPostLoadCallback(localizationGuid, ItemflavWindow_LocalizationPostLoadCallback);
        }

        CPakAsset* const itemflavListAsset = reinterpret_cast<CPakAsset*>(uiState->itemFlavorListAsset);
        const SettingsAsset* const itemflavList = itemflavListAsset->extraData<SettingsAsset*>();

        SettingsKVValue_t* value = nullptr;
        if (itemflavList->GetSettingValue("characters", &value))
        {
            const uint32_t numCharacters = value->numChildren;
            const SettingsKVValue_t* const arrayElems = value->getValue<SettingsKVValue_t*>();

            flavData->AllocCharacterData(numCharacters);

            for (uint32_t j = 0; j < numCharacters; ++j)
            {
                const SettingsKVField_t* const elemFields = arrayElems[j].getValue<SettingsKVField_t*>();

                assert(arrayElems[j].numChildren > 0);
                if (arrayElems[j].numChildren == 0)
                    break;

                CUI_ItemflavCharacter* const character = &flavData->characterData[j];

                character->assetPath = elemFields[0].getValue<const char*>();
                character->settingsAssetGuid = RTech::StringToGuid(character->assetPath);

                // If the settings asset is already loaded, try and populate the data now
                if (CAsset* settingsAsset = g_assetData.FindAssetByGUID(character->settingsAssetGuid))
                    ItemflavWindow_GetCharacterDataFromSettings(settingsAsset);
                else
                    g_assetData.AddAssetPostLoadCallback(character->settingsAssetGuid, ItemflavWindow_GetCharacterDataFromSettings);
            }
        }
    }
}

void ItemflavWnd_Draw(CUIState* uiState)
{
    ImGui::SetNextWindowSize(ImVec2(0.f, 500.f), ImGuiCond_Always);
    if (ImGui::Begin("Skin Finder", &uiState->itemflavWindowVisible, ImGuiWindowFlags_NoCollapse))
    {
        ImGui::Text("This menu contains a list of all registered cosmetic items associated with each game character.\n"
            "For all features to work properly, RSX must load common.rpak, common_early.rpak, and your chosen localization rpak.");

        CUI_ItemflavWindowData* flavData = &uiState->itemflavData;

        // Always render the refresh button, but also try to grab everything the first time the window is shown
        if ((ImGui::Button("Refresh data") || !flavData->triedToInitialise) && g_assetData.m_donePostLoad)
        {
            ItemflavWindow_RefreshData(uiState);

            flavData->triedToInitialise = true;
        }

        if (flavData->localizationAsset)
        {
            if (ImGui::BeginCombo("##characters", flavData->selectedCharacterName.c_str()))
            {
                for (int i = 0; i < flavData->numCharacters; ++i)
                {
                    CUI_ItemflavCharacter* const character = &flavData->characterData[i];
                    bool isSelected = i == uiState->itemflavData.selectedCharacterIdx;

                    const std::string charName = Localize(character->characterName);

                    if (ImGui::Selectable(charName.c_str(), &isSelected))
                    {
                        flavData->selectedCharacterIdx = i;
                        flavData->selectedCharacterName = charName;
                    }

                    if (isSelected)
                        ImGui::SetItemDefaultFocus();
                }

                ImGui::EndCombo();
            }

            if (flavData->selectedCharacterIdx != -1)
            {
                CUI_ItemflavCharacter* character = &flavData->characterData[flavData->selectedCharacterIdx];
                ImGui::TextUnformatted(Localize(character->characterDesc).c_str());

                ImGui::Text("Skins:");

                if (ImGui::BeginTable("SkinsTable", 4, ImGuiTableFlags_BordersOuter))
                {
                    ImGui::TableSetupColumn("Quality", ImGuiTableColumnFlags_WidthFixed, 100.f, 0);
                    ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthFixed, 0, 1);
                    ImGui::TableSetupColumn("Arms Model", ImGuiTableColumnFlags_WidthFixed, 0, 2);
                    ImGui::TableSetupColumn("Body Model", ImGuiTableColumnFlags_WidthFixed, 0, 3);
                    //ImGui::TableSetupColumn("RPak File", 0, 0, 4);

                    ImGui::TableSetupScrollFreeze(0, 1);
                    ImGui::TableHeadersRow();

                    const char* prevPakName = nullptr;

                    size_t i = 0;
                    for (auto& skin : character->skins)
                    {
                        SettingsAsset* const skinSettings = reinterpret_cast<CPakAsset*>(skin.settingsAsset)->extraData<SettingsAsset*>();

                        // _shared_favorites
                        if (skinSettings->uniqueId == 0x053a2537)
                            continue;

                        ImGui::PushID(static_cast<int>(i));

                        ImGui::TableNextRow();

                        if (!prevPakName || _stricmp(skin.armsModelPak, prevPakName))
                        {
                            const float x = ImGuiExt::TableFullRowBegin();

                            //ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 25.f);
                            //if(prevPakName) ImGui::SetCursorPosY(ImGui::GetCursorPosY() + ImGui::GetTextLineHeight());
                            ImGui::SeparatorText(skin.armsModelPak);

                            ImGuiExt::TableFullRowEnd(x);

                            ImGui::TableNextRow();
                        }

                        const std::unordered_map<char, ImVec4> qualityColours = {
                            {'C', ImVec4(0.765f, 0.847f, 0.851f, 1.f)}, // COMMON:    <195,216,217>
                            {'R', ImVec4(0.384f, 0.784f, 1.f, 1.f)},    // RARE:      <98,200,255>
                            {'E', ImVec4(0.784f, 0.302f, 1.f, 1.f)},    // EPIC:      <200,77,255>
                            {'L', ImVec4(1.f, 0.804f, 0.235f, 1.f)},    // LEGENDARY: <255,205,60>
                            {'M', ImVec4(1.f, 0.231f, 0.263f, 1.f)},    // MYTHIC:    <255,59,67>
                            {'I', ImVec4(0.f, 1.f, 0.8f, 1.f)},         // ICONIC:    <0, 255, 204>
                        };

                        const std::string skinName = Localize(skin.localizationKey_NAME);

                        if (ImGui::TableSetColumnIndex(0))
                        {
                            assert(qualityColours.contains(skin.quality[0]));
                            ImGui::PushStyleColor(ImGuiCol_Text, qualityColours.at(skin.quality[0]));
                            
                            ImGui::TextUnformatted(skin.quality);

                            ImGui::PopStyleColor();
                        }

                        if (ImGui::TableSetColumnIndex(1))
                            ImGui::Selectable(skinName.c_str(), false, ImGuiSelectableFlags_SpanAllColumns);

                        if (ImGui::TableSetColumnIndex(2))
                            ImGui::TextUnformatted(GetStringAfterLastSlash(skin.armsModel));

                        if (ImGui::TableSetColumnIndex(3))
                            ImGui::TextUnformatted(GetStringAfterLastSlash(skin.bodyModel));

                        //if (ImGui::TableSetColumnIndex(4))
                        //    ImGui::TextUnformatted(skin.armsModelPak);

                        ImGui::PopID();

                        i++;

                        prevPakName = skin.armsModelPak;
                    }
                    ImGui::EndTable();
                };
            }
        }
    }
    ImGui::End();
}
