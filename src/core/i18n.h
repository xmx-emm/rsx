#pragma once

// Lightweight UI localization layer.
//
// Usage:
//   TR("Settings")      -> translated text for the current language (falls back to the
//                          english source string when no translation is available).
//   TRW("Asset List")   -> window title with a stable "###" id suffix so that dock layout
//                          and imgui.ini data survive language switches. For english this
//                          returns the original pointer, so existing behaviour is unchanged.
//   TRCombo(...)        -> drop-in replacement for ImGui::Combo that translates each item
//                          through TR() before drawing.
//
// Adding a language only requires a new entry in eUILanguage and a new string table in
// i18n.cpp; untranslated strings gracefully fall back to english.
//
// NOTE: all translated literals are UTF-8, the project is compiled with /utf-8.

enum class eUILanguage : uint32_t
{
    UILANG_ENGLISH = 0,
    UILANG_CHINESE_SIMPLIFIED,

    UILANG_COUNT,
};

// display names shown in the language selector (kept in their native language on purpose)
static const char* const s_UILanguageNames[static_cast<uint32_t>(eUILanguage::UILANG_COUNT)] =
{
    "English",
    "简体中文",
};

void I18N_SetLanguage(const eUILanguage language);
eUILanguage I18N_GetLanguage();

// translate a string for the current language, returns the input pointer when no
// translation exists (or when the current language is english). returned pointers have
// static storage duration and stay valid for the lifetime of the program.
const char* TR(const char* const text);

// translate a window title while keeping its ImGui id stable across languages by
// appending "###<english name>". returns the input pointer for english.
const char* TRW(const char* const windowName);

// ImGui::Combo wrapper that runs every item through TR()
bool TRCombo(const char* const label, int* const currentItem, const char* const* const items, const int itemCount);
