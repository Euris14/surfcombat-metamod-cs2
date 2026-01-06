#include "common.h"
#include "utils/utils.h"
#include "surf.h"
#include "utils/simplecmds.h"
#include "utils/eventlistener.h"
#include "surf/sc_features.h"
#include "surf/sc_rank.h"
#include "surf/sc_rtv.h"
#include "surf/sc_privilege.h"

#include <algorithm>

#include "tier0/memdbgon.h"

/*
CON_COMMAND_F(sc_spawnprot_color, "Set Spawn Protection color for player", FCVAR_SERVER_CAN_EXECUTE | FCVAR_LINKED_CONCOMMAND)
{
	if (args.ArgC() != 3)
	{
		ConColorMsg(Color(255, 0, 0, 255), "Usage: sc_spawnprot_color <r> <g> <b>");
		return;
	}

	int r = V_StringToInt32(args[0], 255);
	int g = V_StringToInt32(args[1], 255);
	int b = V_StringToInt32(args[2], 255);

	g_SpawnProtection.SetSpawnColor(r, g, b);
}

CON_COMMAND_F(sc_spawnprot_time, "Set Spawn Protection time duration", FCVAR_SERVER_CAN_EXECUTE | FCVAR_LINKED_CONCOMMAND)
{
	if (args.ArgC() != 1)
	{
		ConColorMsg(Color(255, 0, 0, 255), "Usage: sc_spawnprot_time <time>");
		return;
	}

	int time = V_StringToInt32(args[0], 10);

	g_SpawnProtection.SetTimeProtection(time);
}
*/

internal SCMD_CALLBACK(Command_SCHide)
{
	SURFPlayer *player = SURF::GetSURFPlayerManager()->ToPlayer(controller);
	player->ToggleHide();
	return MRES_SUPERCEDE;
}

internal SCMD_CALLBACK(Command_SCLegs)
{
	SURFPlayer *player = SURF::GetSURFPlayerManager()->ToPlayer(controller);
	player->ToggleHideLegs();
	utils::PrintChat(controller, " \6[SURF] Legs %s!", player->hidePlayerLegs ? "hidden" : "shown");
	return MRES_SUPERCEDE;
}

internal SCMD_CALLBACK(Command_SCResetScore)
{
	controller->m_pActionTrackingServices()->m_matchStats().m_iKills = 0;
	controller->m_pActionTrackingServices()->m_matchStats().m_iAssists = 0;
	controller->m_pActionTrackingServices()->m_matchStats().m_iDeaths = 0;
	controller->m_pActionTrackingServices()->m_matchStats().m_iDamage = 0;
	controller->m_iScore = 0;
	controller->m_iMVPs = 0;

	utils::EntityCollisionRulesChanged(controller->m_hPawn());

	utils::ClientPrint(controller, MsgDest::HUD_PRINTTALK, " \6[SURF] Your scores have been reset!");
	return MRES_SUPERCEDE;
}

// Helper function to find a player by name or steam_id
internal CCSPlayerController* FindPlayerByNameOrSteamID(const char* identifier)
{
	for (int i = 1; i <= MAXPLAYERS; i++)
	{
		CBaseEntity *entity = g_pEntitySystem->GetBaseEntity(CEntityIndex(i));
		if (!entity) continue;
		
		if (utils::IsEntityController(entity))
		{
			CCSPlayerController *playerController = (CCSPlayerController*)entity;
			if (!playerController) continue;

			// Try matching by player name
			const char *playerName = playerController->GetPlayerName();
			if (playerName && V_stristr(playerName, identifier) != nullptr)
			{
				return playerController;
			}

			// Try matching by steam ID
			uint64_t playerSteamID = playerController->m_steamID();
			if (playerSteamID != 0)
			{
				char steamBuf[32];
				V_snprintf(steamBuf, sizeof(steamBuf), "%llu", static_cast<unsigned long long>(playerSteamID));
				if (V_stristr(steamBuf, identifier) != nullptr)
				{
					return playerController;
				}
			}
		}
	}
	return nullptr;
}

internal SCMD_CALLBACK(Command_SCKick)
{
	if (args->ArgC() < 2)
	{
		utils::ClientPrint(controller, MsgDest::HUD_PRINTTALK, " \6[SURF] Usage: !kick <name/steamid> <reason>");
		return MRES_SUPERCEDE;
	}

	CCSPlayerController *targetPlayer = FindPlayerByNameOrSteamID(args->Arg(1));
	if (!targetPlayer)
	{
		utils::ClientPrint(controller, MsgDest::HUD_PRINTTALK, " \6[SURF] Player not found!");
		return MRES_SUPERCEDE;
	}

	const char *reason = args->ArgC() > 2 ? args->Arg(2) : "No reason provided";
	
	// Log the kick
	Message("Player %s kicked %s. Reason: %s\n", controller->GetPlayerName(), targetPlayer->GetPlayerName(), reason);

	// Kick using player slot index (1-based) for kickid command
	CPlayerSlot targetSlot = utils::GetEntityPlayerSlot(targetPlayer);
	int slotIndex = targetSlot.Get() + 1;
	char kickCmd[128];
	V_snprintf(kickCmd, sizeof(kickCmd), "kickid %d %s\n", slotIndex, reason);
	interfaces::pEngine->ServerCommand(nullptr, kickCmd);

	return MRES_SUPERCEDE;
}

internal SCMD_CALLBACK(Command_SCMap)
{
	if (args->ArgC() < 2)
	{
		utils::ClientPrint(controller, MsgDest::HUD_PRINTTALK, " \6[SURF] Usage: !map <mapname>");
		return MRES_SUPERCEDE;
	}

	const char *mapName = args->Arg(1);
	
	// Log the map change
	Message("Player %s changed map to %s\n", controller->GetPlayerName(), mapName);

	// Change the map
	char changeCmd[128];
	V_snprintf(changeCmd, sizeof(changeCmd), "changelevel %s\n", mapName);
	interfaces::pEngine->ServerCommand(nullptr, changeCmd);

	return MRES_SUPERCEDE;
}

internal SCMD_CALLBACK(Command_SCBan)
{
	if (args->ArgC() < 3)
	{
		utils::ClientPrint(controller, MsgDest::HUD_PRINTTALK, " \6[SURF] Usage: !ban <name/steamid> <reason> [time in seconds, -1 for perm]");
		return MRES_SUPERCEDE;
	}

	CCSPlayerController *targetPlayer = FindPlayerByNameOrSteamID(args->Arg(1));
	if (!targetPlayer)
	{
		utils::ClientPrint(controller, MsgDest::HUD_PRINTTALK, " \6[SURF] Player not found!");
		return MRES_SUPERCEDE;
	}

	const char *reason = args->Arg(2);
	int banTime = args->ArgC() > 3 ? V_StringToInt32(args->Arg(3), 0) : 0;

	// Log the ban
	if (banTime == -1)
	{
		Message("Player %s permanently banned %s. Reason: %s\n", controller->GetPlayerName(), targetPlayer->GetPlayerName(), reason);
	}
	else
	{
		Message("Player %s banned %s for %d seconds. Reason: %s\n", controller->GetPlayerName(), targetPlayer->GetPlayerName(), banTime, reason);
	}

	// Execute ban command: duration in minutes (0 = permanent), player slot is 1-based
	int banMinutes = (banTime == -1) ? 0 : (banTime / 60);
	CPlayerSlot targetSlot = utils::GetEntityPlayerSlot(targetPlayer);
	int slotIndex = targetSlot.Get() + 1;
	char banCmd[160];
	V_snprintf(banCmd, sizeof(banCmd), "banid %d %d %s\n", banMinutes, slotIndex, reason);
	interfaces::pEngine->ServerCommand(nullptr, banCmd);

	return MRES_SUPERCEDE;
}

internal SCMD_CALLBACK(Command_SCGag)
{
	if (args->ArgC() < 3)
	{
		utils::ClientPrint(controller, MsgDest::HUD_PRINTTALK, " \6[SURF] Usage: !gag <name/steamid> <reason> [time in seconds]");
		return MRES_SUPERCEDE;
	}

	CCSPlayerController *targetPlayer = FindPlayerByNameOrSteamID(args->Arg(1));
	if (!targetPlayer)
	{
		utils::ClientPrint(controller, MsgDest::HUD_PRINTTALK, " \6[SURF] Player not found!");
		return MRES_SUPERCEDE;
	}

	const char *reason = args->Arg(2);
	int gagTime = args->ArgC() > 3 ? V_StringToInt32(args->Arg(3), 300) : 300; // Default 5 minutes

	// Log the gag
	Message("Player %s gagged %s for %d seconds. Reason: %s\n", controller->GetPlayerName(), targetPlayer->GetPlayerName(), gagTime, reason);

	// Note: Gagging requires a plugin or extension. For now, we'll just log it.
	utils::ClientPrint(targetPlayer, MsgDest::HUD_PRINTTALK, " \6[SURF] You have been gagged!");

	return MRES_SUPERCEDE;
}

internal SCMD_CALLBACK(Command_SCWS)
{
	CCSPlayerPawnBase* pPawn = controller->m_hPawn();
	if (!pPawn || pPawn->m_lifeState() != LIFE_ALIVE)
	{
		utils::ClientPrint(controller, MsgDest::HUD_PRINTTALK, " \6[SURF] You must be alive to check weapon stats!");
		return MRES_SUPERCEDE;
	}

	utils::ClientPrint(controller, MsgDest::HUD_PRINTTALK, " \6[SURF] Weapon stats are not available right now.");

	return MRES_SUPERCEDE;
}

internal SCMD_CALLBACK(Command_SCRank)
{
	int playerIndex = controller->entindex();
	PlayerRank* pRank = g_RankSystem.GetPlayerRank(playerIndex);
	
	if (!pRank)
	{
		utils::ClientPrint(controller, MsgDest::HUD_PRINTTALK, " \6[SURF] Error retrieving your rank!");
		return MRES_SUPERCEDE;
	}

	utils::PrintAlert(controller, "=== Your Rank ===\nRank: %s\nRating: %.0f\nWins: %d\n", 
		pRank->GetRankName(), pRank->GetRating(), pRank->GetWins());
	utils::PrintChat(controller, " \6[SURF] %s | Rating: %.0f", pRank->GetRankName(), pRank->GetRating());

	return MRES_SUPERCEDE;
}

internal SCMD_CALLBACK(Command_SCLeaderboard)
{
	// Display top 5 players by rating
	utils::PrintAlert(controller, "=== Top 5 Players ===\n");
	
	struct PlayerScore
	{
		const char* name;
		float rating;
		const char* rank;
	};
	
	std::vector<PlayerScore> scores;
	
	for (int i = 1; i <= MAXPLAYERS; i++)
	{
		CBaseEntity *entity = g_pEntitySystem->GetBaseEntity(CEntityIndex(i));
		if (!entity || !utils::IsEntityController(entity)) continue;
		
		CCSPlayerController *playerController = (CCSPlayerController*)entity;
		PlayerRank* pRank = g_RankSystem.GetPlayerRank(i);
		
		if (pRank && pRank->GetRating() > 0)
		{
			scores.push_back({playerController->GetPlayerName(), pRank->GetRating(), pRank->GetRankName()});
		}
	}
	
	// Sort by rating
	std::sort(scores.begin(), scores.end(), [](const PlayerScore& a, const PlayerScore& b) {
		return a.rating > b.rating;
	});
	
	// Print top 5
	for (int i = 0; i < std::min(5, static_cast<int>(scores.size())); i++)
	{
		utils::PrintAlert(controller, "%d. %s - %s (%.0f)\n", i + 1, scores[i].name, scores[i].rank, scores[i].rating);
	}

	return MRES_SUPERCEDE;
}

internal SCMD_CALLBACK(Command_SCPrivilege)
{
	int playerIndex = controller->entindex();
	PlayerPrivilege* pPrivilege = g_PrivilegeSystem.GetPlayerPrivilege(playerIndex);
	
	if (!pPrivilege || !pPrivilege->IsPrivilegeActive())
	{
		utils::ClientPrint(controller, MsgDest::HUD_PRINTTALK, " \6[SURF] You have no active privileges. Visit GameCMS or Telegram bot to purchase!");
		return MRES_SUPERCEDE;
	}

	float expiryTime = pPrivilege->GetExpiryTime();
	if (expiryTime < 0.0f)
	{
		utils::PrintAlert(controller, "=== Your Privilege ===\nLevel: %s\nExpiry: Permanent\n", pPrivilege->GetPrivilegeName());
	}
	else
	{
		float remaining = expiryTime - utils::GetServerGlobals()->curtime;
		utils::PrintAlert(controller, "=== Your Privilege ===\nLevel: %s\nTime remaining: %.0f seconds\n", pPrivilege->GetPrivilegeName(), remaining);
	}
	
	utils::PrintChat(controller, " \6[SURF] Privilege: %s", pPrivilege->GetPrivilegeName());

	return MRES_SUPERCEDE;
}

internal SCMD_CALLBACK(Command_SCRockTheVote)
{
	if (!g_RockTheVote.IsVoteActive())
	{
		g_RockTheVote.StartVote();
	}
	else
	{
		utils::ClientPrint(controller, MsgDest::HUD_PRINTTALK, " \6[SURF] A vote is already in progress!");
	}
	return MRES_SUPERCEDE;
}

internal SCMD_CALLBACK(Command_SCVote)
{
	if (!g_RockTheVote.IsVoteActive())
	{
		utils::ClientPrint(controller, MsgDest::HUD_PRINTTALK, " \6[SURF] No vote in progress! Use !rtv to start one.");
		return MRES_SUPERCEDE;
	}

	if (args->ArgC() < 2)
	{
		utils::ClientPrint(controller, MsgDest::HUD_PRINTTALK, " \6[SURF] Usage: !vote <map number>");
		
		// Show available maps
		const auto& maps = g_RockTheVote.GetMaps();
		for (int i = 0; i < (int)maps.size() && i < 8; i++)
		{
			utils::PrintChat(controller, " \6[SURF] %d. %s (%d votes)", i + 1, maps[i].mapName.c_str(), maps[i].votes);
		}
		return MRES_SUPERCEDE;
	}

	int mapIndex = V_StringToInt32(args->Arg(1), -1) - 1;
	int playerIndex = controller->entindex();

	g_RockTheVote.AddVote(playerIndex, mapIndex);

	const auto& maps = g_RockTheVote.GetMaps();
	if (mapIndex >= 0 && mapIndex < (int)maps.size())
	{
		utils::PrintChat(controller, " \6[SURF] You voted for %s!", maps[mapIndex].mapName.c_str());
	}

	return MRES_SUPERCEDE;
}

void SURF::misc::RegisterCommands()
{
	scmd::RegisterCmd("hide",	Command_SCHide);
	scmd::RegisterCmd("legs",	Command_SCLegs);
	scmd::RegisterCmd("rs",		Command_SCResetScore);
	scmd::RegisterCmd("kick",	Command_SCKick);
	scmd::RegisterCmd("ban",	Command_SCBan);
	scmd::RegisterCmd("gag",	Command_SCGag);
	scmd::RegisterCmd("map",	Command_SCMap);
	scmd::RegisterCmd("ws",		Command_SCWS);
	scmd::RegisterCmd("rank",	Command_SCRank);
	scmd::RegisterCmd("top",	Command_SCLeaderboard);
	scmd::RegisterCmd("rtv",	Command_SCRockTheVote);
	scmd::RegisterCmd("vote",	Command_SCVote);
	scmd::RegisterCmd("privilege", Command_SCPrivilege);
	scmd::RegisterCmd("priv",	Command_SCPrivilege);
}

void SURF::misc::OnCheckTransmit(CCheckTransmitInfo **pInfo, int infoCount)
{
	for (int i = 0; i < infoCount; i++)
	{
		// Cast it to our own TransmitInfo struct because CCheckTransmitInfo isn't correct.
		TransmitInfo *pTransmitInfo = reinterpret_cast<TransmitInfo*>(pInfo[i]);
		
		// Find out who this info will be sent to.
		CPlayerSlot targetSlot = pTransmitInfo->m_nClientEntityIndex;
		SURFPlayer *targetPlayer = SURF::GetSURFPlayerManager()->ToPlayer(targetSlot);
		
		// Don't hide if player is dead/spectating or if they aren't hiding other players.
		if (!targetPlayer->hideOtherPlayers) continue;
		if (targetPlayer->GetPawn()->m_lifeState() != LIFE_ALIVE) continue;
		// Loop through the list of players and see if they need to be hidden away from our target player.
		for (int j = 0; j < MAXPLAYERS; j++)
		{
			if (j == targetPlayer->GetController()->entindex()) continue;
			CBasePlayerPawn *pawn = SURF::GetSURFPlayerManager()->players[j]->GetPawn();
			if (!pawn) continue;

			// Hide only teammates
			if (targetPlayer->GetController()->m_iTeamNum.Get() == pawn->m_iTeamNum.Get())
			{
				if (pTransmitInfo->m_pTransmitEdict->IsBitSet(pawn->entindex()))
				{
					pTransmitInfo->m_pTransmitEdict->Clear(pawn->entindex());
				}
			}
		}
	}
}

void SURF::misc::OnClientPutInServer(CPlayerSlot slot)
{
	SURFPlayer *player = SURF::GetSURFPlayerManager()->ToPlayer(slot);
	player->Reset();
}