#include "movement.h"
#include "utils/utils.h"

#include "tier0/memdbgon.h"
extern CEntitySystem *g_pEntitySystem;

static inline bool IsPlayerIndexInRange(int index)
{
	return index >= 0 && index <= MAXPLAYERS;
}

static inline MovementPlayer *PlayerByIndex(MovementPlayer *const *players, int index)
{
	if (!IsPlayerIndexInRange(index))
		return nullptr;
	return players[index];
}

MovementPlayer *CMovementPlayerManager::ToPlayer(CCSPlayer_MovementServices *ms)
{
	if (!ms || !ms->pawn)
		return nullptr;
	return PlayerByIndex(this->players, ms->pawn->m_hController().GetEntryIndex());
}

MovementPlayer *CMovementPlayerManager::ToPlayer(CCSPlayerController *controller)
{
	if (!controller || !controller->m_pEntity)
		return nullptr;
	return PlayerByIndex(this->players, controller->m_pEntity->m_EHandle.GetEntryIndex());
}

MovementPlayer *CMovementPlayerManager::ToPlayer(CBasePlayerPawn *pawn)
{
	if (!pawn)
		return nullptr;
	return PlayerByIndex(this->players, pawn->m_hController().GetEntryIndex());
}

MovementPlayer *CMovementPlayerManager::ToPlayer(CPlayerSlot slot)
{
	return PlayerByIndex(this->players, slot.Get() + 1);
}

MovementPlayer *CMovementPlayerManager::ToPlayer(CEntityIndex entIndex)
{
	if (!g_pEntitySystem) return nullptr;
	CBaseEntity *ent = g_pEntitySystem->GetBaseEntity(entIndex);
	if (!ent || !ent->m_pEntity)
		return nullptr;
	return PlayerByIndex(this->players, ent->m_pEntity->m_EHandle.GetEntryIndex());
}

MovementPlayer *CMovementPlayerManager::ToPlayer(CPlayerUserId userID)
{
	if (!g_pEntitySystem || !interfaces::pEngine) return nullptr;
	for (int i = 0; i < MAXPLAYERS; i++)
	{
		if (interfaces::pEngine->GetPlayerUserId(i) == userID.Get())
		{
			return PlayerByIndex(this->players, i + 1);
		}
	}
	return nullptr;
}