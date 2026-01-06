#include "sc_event_manager.h"
#include "sc_features.h"
#include "sc_rank.h"
#include <algorithm>
#include <string>

std::unordered_map<EventID, Event*> g_umpEventManager;
std::queue<std::function<void()>> g_qEventsInit;

GAME_EVENT(player_spawn, EventID::PLAYER_SPAWN)
{
	CBasePlayerController* pController = static_cast<CBasePlayerController*>(pEvent->GetPlayerController("userid"));

	if (!pController)
		return false;

	CCSPlayerPawnBase* pPawn = pController->m_hPawn();
	if (!pPawn || (pPawn->m_lifeState() != LifeState_t::LIFE_ALIVE))
		return false;

	g_SurfPlugin.NextFrame([pPawn]()
		{
			pPawn->m_bTakesDamage(false);
			pPawn->m_clrRender(g_SpawnProtection.GetSpawnColor());
			pPawn->m_pCollision->m_CollisionGroup = COLLISION_GROUP_DEBRIS;
			pPawn->m_pCollision->m_collisionAttribute().m_nCollisionGroup = COLLISION_GROUP_DEBRIS;
			utils::EntityCollisionRulesChanged(pPawn);
		});

	utils::PrintCentre(pController, "Spawn Protection %d sec.", (int)g_SpawnProtection.GetTimeProtection());
	
	new CTimer(g_SpawnProtection.GetTimeProtection() + 1.0f, false, [hController = CHandle<CBasePlayerController>(pController)]()
		{

			CCSPlayerController* pController = static_cast<CCSPlayerController*>(hController.Get());
			if (!pController)
				return -1.0f;

			CCSPlayerPawnBase* pPawn = pController->m_hPawn();
			if (!pPawn || (pPawn->m_lifeState() != LifeState_t::LIFE_ALIVE))
				return -1.0f;

			Color colorteam;
			int alpha = pPawn->m_clrRender().a();

			if (pPawn->m_iTeamNum == CS_TEAM_CT)	 colorteam.SetColor(0, 0, 255, alpha);
			else if (pPawn->m_iTeamNum == CS_TEAM_T) colorteam.SetColor(255, 0, 0, alpha);
			else return -1.0f;

			utils::PrintCentre(pController, "Protection is gone");
			pPawn->m_clrRender(colorteam);
			pPawn->m_bTakesDamage(true);

			return -1.0f;
		});

	return true;
}

GAME_EVENT(player_death, EventID::PLAYER_DEATH)
{
	CBasePlayerController* pVictimController = static_cast<CBasePlayerController*>(pEvent->GetPlayerController("userid"));
	CBasePlayerController* pKillerController = static_cast<CBasePlayerController*>(pEvent->GetPlayerController("attacker"));

	if (!pKillerController || !pVictimController)
		return false;

	CCSPlayerController* pKillerCsController = static_cast<CCSPlayerController*>(pKillerController);

	// Make sure it was actually a kill (not suicide)
	if (pKillerController == pVictimController)
		return false;

	CCSPlayerPawnBase* pKillerPawn = pKillerController->m_hPawn();
	if (!pKillerPawn || pKillerPawn->m_lifeState() != LifeState_t::LIFE_ALIVE)
		return false;

	// Get damage info from event
	int damage = pEvent->GetInt("dmg_health");
	const char *weapon = pEvent->GetString("weapon");

	// Give killer 10-5 HP based on random value
	int hpGain = 5 + (rand() % 6); // Random between 5-10
	int currentHealth = pKillerPawn->m_iHealth();
	const int maxHealth = 100;
	int newHealth = std::min(currentHealth + hpGain, maxHealth);
	pKillerPawn->m_iHealth(newHealth);

	// Update killer's rank
	int killerIndex = pKillerController->entindex();
	g_RankSystem.UpdatePlayerStats(killerIndex, false, 1);
	PlayerRank* pKillerRank = g_RankSystem.GetPlayerRank(killerIndex);

	// Send kill bonus message with rank info
	std::string killerName = pKillerCsController ? pKillerCsController->GetPlayerName() : "Unknown";

	g_SurfPlugin.NextFrame([pKillerController, damage, weapon, pKillerRank, killerName]()
		{
			utils::PrintCentre(pKillerController, "Kill! Bonus +5-10 HP! Damage: %d (%s)", damage, weapon);
			utils::PrintAlert(pKillerController, "Killed! Rating: %.0f | Rank: %s | Damage dealt: %d\n", 
				pKillerRank->GetRating(), pKillerRank->GetRankName(), damage);
		});

	// Send death message to victim
	g_SurfPlugin.NextFrame([pVictimController, damage, weapon, pKillerRank, killerName]()
		{
			utils::PrintAlert(pVictimController, "Killed by %s (%s)! Damage taken: %d (%s)\n", 
				killerName.c_str(), pKillerRank->GetRankName(), damage, weapon);
		});

	return true;
}

void Init_EventManager()
{
	while (!g_qEventsInit.empty())
	{
		g_qEventsInit.front()();
		g_qEventsInit.pop();
	}
}