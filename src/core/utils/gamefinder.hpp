#pragma once
#include "ValveFileVDF/vdf_parser.hpp"

const std::unordered_map<std::string, const char*> STEAM_APP_IDS = {
	//{"1454890", "Titanfall"}, // Titanfall - we're not quite there yet with R1 support so let's hold off for now!
	{"1237970", "Titanfall2"}, // Titanfall 2
	{"1172470", "Apex Legends"}, // Apex Legends
};

inline std::vector<std::filesystem::path> GameFinder_FindAllCompatibleSteamGames()
{
	wchar_t buf[1024] = {};
	DWORD bufSize = 1024;

	if (RegGetValueW(HKEY_CURRENT_USER, L"Software\\Valve\\Steam", L"SteamPath", RRF_RT_ANY, NULL, &buf, &bufSize))
	{
		Log("GameFinder: Failed to locate steam installation\n");
		return {};
	}

	std::filesystem::path steamInstallationPath = buf;
	
	if (!std::filesystem::exists(steamInstallationPath / "steamapps" / "libraryfolders.vdf"))
	{
		Log("GameFinder: Failed to locate libraryfolders.vdf within steamapps directory\n");
		return {};
	}

	std::ifstream lfStream(steamInstallationPath / "steamapps" / "libraryfolders.vdf", std::ios::in);

	if (!lfStream.is_open())
	{
		Log("GameFinder: Failed to open libraryfolders.vdf\n");
		return {};
	}

	/*
	"libraryfolders"
	{
		"0"
		{
			"path" "C:\\Program Files (x86)\\Steam"
			// ...
			"apps"
			{
				"appid1" "size"
				"appid2" "size"
				// ...
			}
		}
	}
	*/

	std::vector<std::filesystem::path> gameDirs = {};

	auto root = tyti::vdf::read(lfStream);

	assert(root.name == "libraryfolders");

	for (auto& child : root.childs)
	{
		assert(child.second->childs.contains("apps"));

		std::filesystem::path libraryPath = child.second->attribs["path"];
		libraryPath /= "steamapps\\common";

		for (auto& app : child.second->childs["apps"]->attribs)
		{
			if (auto it = STEAM_APP_IDS.find(app.first); it != STEAM_APP_IDS.end())
			{
				std::filesystem::path gamePath = libraryPath / it->second;

				// Final check to make sure that steam isn't lying to us
				if(std::filesystem::exists(gamePath))
					gameDirs.push_back(gamePath);
			}
		}
	}


	return gameDirs;
}
