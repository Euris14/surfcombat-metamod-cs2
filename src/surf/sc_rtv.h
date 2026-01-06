#pragma once

#include "common.h"
#include <vector>
#include <string>

struct RockTheVoteMap
{
	std::string mapName;
	int votes;
};

class RockTheVoteSystem
{
public:
	RockTheVoteSystem();
	
	void StartVote();
	void AddVote(int playerIndex, int mapIndex);
	void ExecuteVote();
	void Reset();
	
	const std::vector<RockTheVoteMap>& GetMaps() const { return m_maps; }
	bool IsVoteActive() const { return m_bVoteActive; }
	float GetRemainingTime() const { return m_flVoteEndTime - utils::GetServerGlobals()->curtime; }

private:
	std::vector<RockTheVoteMap> m_maps;
	bool m_bVoteActive;
	float m_flVoteEndTime;
	int m_iPlayerVotes[MAXPLAYERS + 1]; // Track which map each player voted for
};

extern RockTheVoteSystem g_RockTheVote;
