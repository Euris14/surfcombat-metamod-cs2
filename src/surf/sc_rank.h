#pragma once

#include "common.h"

enum RankType
{
	RANK_UNRANKED = 0,
	RANK_SILVER_1,
	RANK_SILVER_2,
	RANK_SILVER_3,
	RANK_SILVER_4,
	RANK_SILVER_ELITE,
	RANK_GOLD_NOVA_1,
	RANK_GOLD_NOVA_2,
	RANK_GOLD_NOVA_3,
	RANK_GOLD_NOVA_MASTER,
	RANK_MASTER_GUARDIAN_1,
	RANK_MASTER_GUARDIAN_2,
	RANK_MASTER_GUARDIAN_ELITE,
	RANK_DISTINGUISHED_MASTER_GUARDIAN,
	RANK_LEGENDARY_EAGLE,
	RANK_LEGENDARY_EAGLE_MASTER,
	RANK_SUPREME_MASTER_FIRST_CLASS,
	RANK_GLOBAL_ELITE,
	RANK_MAX
};

class PlayerRank
{
public:
	PlayerRank() : m_rating(0.0f), m_wins(0), m_rank(RANK_UNRANKED), m_kills(0) {}

	void SetRank(RankType rank) { m_rank = rank; }
	RankType GetRank() const { return m_rank; }

	void SetRating(float rating) { m_rating = rating; }
	float GetRating() const { return m_rating; }

	void AddWin() { m_wins++; }
	int GetWins() const { return m_wins; }

	void AddKill() { m_kills++; }
	int GetKills() const { return m_kills; }

	void UpdateRank();
	const char* GetRankName() const;

	// Make these public for accessing in lambda captures
	float m_rating;
	int m_wins;

private:
	RankType m_rank;
	int m_kills;
};

class RankSystem
{
public:
	PlayerRank* GetPlayerRank(int playerIndex);
	void UpdatePlayerStats(int playerIndex, bool bWon, int kills);
	const char* GetRankName(RankType rank) const;

private:
	PlayerRank m_playerRanks[MAXPLAYERS + 1];
};

extern RankSystem g_RankSystem;
