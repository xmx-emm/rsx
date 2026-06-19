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

            // If there's no localization key for this skin, fall back on the "skin name" field.
            // This should really only be applicable for DUMMIE's colour skins, since they are never shown in the regular cosmetics menu
            // and therefore don't need a localized name.
            if (skinNameValue->getValue<const char*>()[0] == '\0')
                skinAsset->GetSettingValue("skinName", &skinNameValue);

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
            skin.includeInList = true;

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
            if (ImGui::BeginCombo("##characters", flavData->selectedCharacterName.c_str(), ImGuiComboFlags_WidthFitPreview))
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
                ImGui::SameLine();
                ImGui::TextUnformatted(Localize(character->characterDesc).c_str());

                ImGui::TextUnformatted("Search"); ImGui::SameLine();

                static ImGuiCustomTextFilter skinFilter;

                // OR case if we load a pak and the filter is not cleared yet.
                if (skinFilter.Draw("##SkinFilter", -1.f))
                {
                    for (auto& skin : character->skins)
                    {
                        if (!skin.localizationKey_NAME) continue;
                        skin.includeInList = (!skinFilter.IsActive()) || skinFilter.PassFilter(Localize(skin.localizationKey_NAME).c_str()) || skinFilter.PassFilter(skin.armsModel) || skinFilter.PassFilter(skin.bodyModel);
                    }
                }

                const char* prevPakName = nullptr;

                bool headerOpen = false;
                bool lastTableRet = false;

                size_t i = 0;
                for (auto& skin : character->skins)
                {
                    i++; // increment here so we don't have to do it in every continue branch

                    SettingsAsset* const skinSettings = reinterpret_cast<CPakAsset*>(skin.settingsAsset)->extraData<SettingsAsset*>();

                    // _shared_favorites is not an actual character skin. fuck bakery
                    if (skinSettings->uniqueId == 0x053a2537)
                        continue;

                    if (!skin.includeInList && skinFilter.IsActive())
                        continue;

                    // If this skin's arms model pak name is different to the one before it, split the following skins into a new group
                    if (!prevPakName || _stricmp(skin.armsModelPak, prevPakName))
                    {
                        // If we aren't the first table, and the last table returned true (and was open), we need to end the previous table
                        if (prevPakName && lastTableRet && headerOpen)
                        {
                            ImGui::EndTable();

                            // Reset LTR to false so that we don't hit the EndTable call at the end of the loop
                            lastTableRet = false;
                        }

                        // If we aren't the first table, and the previous header was open, add padding before this header 
                        if(prevPakName != nullptr && headerOpen)
                            ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 10.f);

                        headerOpen = ImGui::CollapsingHeader(skin.armsModelPak);

                        // If the user has opened the header, setup the table for this group
                        if (headerOpen)
                        {
                            ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 10.f);

                            ImGui::Text("These skins can be found by opening file '%s':", skin.armsModelPak);

                            lastTableRet = ImGui::BeginTableEx("SkinsTable", static_cast<ImGuiID>(i | 0x8000), 4, ImGuiTableFlags_BordersOuter | ImGuiTableFlags_BordersInner);
                            if (lastTableRet)
                            {
                                ImGui::TableSetupColumn("Quality", ImGuiTableColumnFlags_WidthFixed, 100.f, 0);
                                ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthFixed, 0, 1);
                                ImGui::TableSetupColumn("Arms Model", ImGuiTableColumnFlags_WidthFixed, 0, 2);
                                ImGui::TableSetupColumn("Body Model", ImGuiTableColumnFlags_WidthFixed, 0, 3);

                                ImGui::TableSetupScrollFreeze(0, 1);
                                ImGui::TableHeadersRow();
                            }
                        }
                    }

                    ImGui::PushID(static_cast<int>(i));

                    if (headerOpen)
                    {
                        ImGui::TableNextRow();

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
                    }

                    ImGui::PopID();

                    prevPakName = skin.armsModelPak;
                }

                // If the last table was opened successfully, end it!
                if (lastTableRet && headerOpen) ImGui::EndTable();
            }
        }
    }
    ImGui::End();
}
