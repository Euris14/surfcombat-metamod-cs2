#pragma once

#include "ehandle.h"
#include "cbaseentity.h"
#include "ccsplayerpawn.h"

class CBasePlayerController : public CBaseEntity2
{
public:
	DECLARE_SCHEMA_CLASS(CBasePlayerController);

	SCHEMA_FIELD(uint64, m_steamID)
	SCHEMA_FIELD(CHandle<CCSPlayerPawn>, m_hPawn)
	SCHEMA_FIELD(char, m_iszPlayerName)

	const char *GetPlayerName()
	{
		// Schema exposes the start of the player name buffer as a char field
		return reinterpret_cast<const char*>(&m_iszPlayerName());
	}
};