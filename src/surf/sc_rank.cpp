#include "sc_rank.h"

RankSystem g_RankSystem;

const char* RankNames[] = {
	"Unranked",
	"Silver I",
	"Silver II",
	"Silver III",
	"Silver IV",
	"Silver Elite",
	"Gold Nova I",
	"Gold Nova II",
	"Gold Nova III",
	"Gold Nova Master",
	"Master Guardian I",
	"Master Guardian II",
	"Master Guardian Elite",
	"Distinguished Master Guardian",
	"Legendary Eagle",
	"Legendary Eagle Master",
	"Supreme Master First Class",
	"Global Elite"
};

PlayerRank* RankSystem::GetPlayerRank(int playerIndex)
{
	if (playerIndex < 0 || playerIndex > MAXPLAYERS)
		return nullptr;
	return &m_playerRanks[playerIndex];
}

void PlayerRank::UpdateRank()
{
	// Update rank based on rating
	// 100 points per rank
	int rankIndex = (int)(m_rating / 100.0f);
	if (rankIndex >= RANK_MAX - 1)
		rankIndex = RANK_MAX - 1;

	m_rank = (RankType)(rankIndex + RANK_SILVER_1);
}

const char* PlayerRank::GetRankName() const
{
	if (m_rank < RANK_MAX)
		return RankNames[m_rank];
	return "Unknown";
}

const char* RankSystem::GetRankName(RankType rank) const
{
	if (rank < RANK_MAX)
		return RankNames[rank];
	return "Unknown";
}

void RankSystem::UpdatePlayerStats(int playerIndex, bool bWon, int kills)
{
	PlayerRank* pRank = GetPlayerRank(playerIndex);
	if (!pRank)
		return;

	// Add kill rating (1 rating per kill)
	pRank->m_rating += kills;

	// Add win bonus (10 rating per win)
	if (bWon)
	{
		pRank->m_wins++;
		pRank->m_rating += 10.0f;
	}

	// Update rank based on new rating
	pRank->UpdateRank();
}
