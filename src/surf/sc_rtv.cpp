#include "sc_rtv.h"
#include "utils/utils.h"
#include "utils/ctimer.h"
#include <random>

RockTheVoteSystem g_RockTheVote;

RockTheVoteSystem::RockTheVoteSystem() : m_bVoteActive(false), m_flVoteEndTime(0.0f)
{
	// Initialize common surf maps
	m_maps.push_back({"surf_utopia_v3", 0});
	m_maps.push_back({"surf_beginner", 0});
	m_maps.push_back({"surf_greatriver", 0});
	m_maps.push_back({"surf_phantom", 0});
	m_maps.push_back({"surf_ramp", 0});
	m_maps.push_back({"surf_practice", 0});
	m_maps.push_back({"surf_timewarp", 0});
	m_maps.push_back({"surf_rebel", 0});

	// Initialize vote tracking
	for (int i = 0; i <= MAXPLAYERS; i++)
	{
		m_iPlayerVotes[i] = -1;
	}
}

void RockTheVoteSystem::StartVote()
{
	if (m_bVoteActive)
		return; // Vote already active

	m_bVoteActive = true;
	m_flVoteEndTime = utils::GetServerGlobals()->curtime + 30.0f; // 30 second vote

	// Reset votes
	for (auto& map : m_maps)
	{
		map.votes = 0;
	}

	for (int i = 0; i <= MAXPLAYERS; i++)
	{
		m_iPlayerVotes[i] = -1;
	}

	// Announce vote
	utils::PrintChatAll(" \6[SURF] Rock The Vote started! Vote for next map with !rtv <number>");
	
	// Print map options
	for (int i = 0; i < (int)m_maps.size(); i++)
	{
		utils::PrintChatAll(" \6[SURF] %d. %s", i + 1, m_maps[i].mapName.c_str());
	}
}

void RockTheVoteSystem::AddVote(int playerIndex, int mapIndex)
{
	if (!m_bVoteActive)
		return;

	if (mapIndex < 0 || mapIndex >= (int)m_maps.size())
		return;

	// Remove previous vote if player voted before
	if (m_iPlayerVotes[playerIndex] != -1)
	{
		m_maps[m_iPlayerVotes[playerIndex]].votes--;
	}

	// Add new vote
	m_iPlayerVotes[playerIndex] = mapIndex;
	m_maps[mapIndex].votes++;
}

void RockTheVoteSystem::ExecuteVote()
{
	int maxVotes = 0;
	std::vector<int> winningMaps;

	// Find map(s) with most votes
	for (int i = 0; i < (int)m_maps.size(); i++)
	{
		if (m_maps[i].votes > maxVotes)
		{
			maxVotes = m_maps[i].votes;
			winningMaps.clear();
			winningMaps.push_back(i);
		}
		else if (m_maps[i].votes == maxVotes && maxVotes > 0)
		{
			winningMaps.push_back(i);
		}
	}

	// Randomly select winner if there's a tie
	int winningMapIndex = winningMaps[0];
	if (winningMaps.size() > 1)
	{
		std::random_device rd;
		std::mt19937 gen(rd());
		std::uniform_int_distribution<> dis(0, winningMaps.size() - 1);
		winningMapIndex = winningMaps[dis(gen)];
	}

	const std::string& winningMap = m_maps[winningMapIndex].mapName;
	
	// Validate map name to prevent command injection (alphanumeric, underscore, hyphen only)
	bool isValid = true;
	for (char c : winningMap)
	{
		if (!isalnum(c) && c != '_' && c != '-')
		{
			isValid = false;
			break;
		}
	}
	
	if (!isValid || winningMap.empty())
	{
		utils::PrintChatAll(" \6[SURF] Error: Invalid map name detected!");
		Reset();
		return;
	}
	
	utils::PrintChatAll(" \6[SURF] Rock The Vote: Next map is %s (%d votes)!", winningMap.c_str().c_str(), maxVotes);

	// Change map
	interfaces::pEngine->ServerCommand(nullptr, UTIL_VarArgs("changelevel %s\n", winningMap.c_str()));

	Reset();
}

void RockTheVoteSystem::Reset()
{
	m_bVoteActive = false;
	m_flVoteEndTime = 0.0f;

	// Reset votes
	for (auto& map : m_maps)
	{
		map.votes = 0;
	}

	for (int i = 0; i <= MAXPLAYERS; i++)
	{
		m_iPlayerVotes[i] = -1;
	}
}
