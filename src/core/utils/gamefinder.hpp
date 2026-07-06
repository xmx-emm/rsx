#pragma once
#include "ValveFileVDF/vdf_parser.hpp"

const std::unordered_map<std::string, const char*> STEAM_APP_IDS = {
	//{"1454890", "Titanfall"}, // Titanfall - we're not quite there yet with R1 support so let's hold off for now!
	{"1237970", "Titanfall2"}, // Titanfall 2
	{"1172470", "Apex Legends"}, // Apex Legends
};

enum class GameFinderGame_e
{
	INVALID = 0,

	TITANFALL_1, // erm actually it's called Titanfall
	TITANFALL_2,

	APEX_LEGENDS = 5, // i can be very funny sometimes
};

struct GameFinderResults_s
{
	std::vector<std::filesystem::path> gamePaths;
	std::vector<GameFinderGame_e> gameTypes;
};

inline bool GameFinder_FindAllCompatibleSteamGames(GameFinderResults_s* results)
{
	assert(results);

	wchar_t buf[1024] = {};
	DWORD bufSize = 1024;

	if (RegGetValueW(HKEY_CURRENT_USER, L"Software\\Valve\\Steam", L"SteamPath", RRF_RT_ANY, NULL, &buf, &bufSize))
	{
		Log("GameFinder: Failed to locate steam installation\n");
		return false;
	}

	std::filesystem::path steamInstallationPath = buf;
	
	if (!std::filesystem::exists(steamInstallationPath / "steamapps" / "libraryfolders.vdf"))
	{
		Log("GameFinder: Failed to locate libraryfolders.vdf within steamapps directory\n");
		return false;
	}

	std::ifstream lfStream(steamInstallationPath / "steamapps" / "libraryfolders.vdf", std::ios::in);

	if (!lfStream.is_open())
	{
		Log("GameFinder: Failed to open libraryfolders.vdf\n");
		return false;
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
				if (std::filesystem::exists(gamePath))
				{
					results->gamePaths.push_back(gamePath);
					
					// Discover the type of game! Sure I could just use the steam app id to determine this
					// but like what if I didn't do that
					if (std::filesystem::exists(gamePath / "r5apexdata.bin"))
						results->gameTypes.push_back(GameFinderGame_e::APEX_LEGENDS);
					else if (std::filesystem::exists(gamePath / "Titanfall2.exe"))
						results->gameTypes.push_back(GameFinderGame_e::TITANFALL_2);
					else if (std::filesystem::exists(gamePath / "Titanfall.exe"))
						results->gameTypes.push_back(GameFinderGame_e::TITANFALL_1);
				}
			}
		}
	}


	return true;
}
