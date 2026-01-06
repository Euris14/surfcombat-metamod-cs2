#include "sc_privilege.h"
#include "utils/utils.h"

PrivilegeSystem g_PrivilegeSystem;

const char* PrivilegeNames[] = {
	"User",
	"Donor",
	"VIP",
	"Admin",
	"Owner"
};

bool PlayerPrivilege::IsPrivilegeActive() const
{
	if (m_privilege == PRIVILEGE_USER)
		return false; // User has no special privilege

	if (m_expiryTime < 0.0f)
		return true; // Permanent privilege

	// Check if privilege has expired
	return utils::GetServerGlobals()->curtime < m_expiryTime;
}

const char* PlayerPrivilege::GetPrivilegeName() const
{
	if (m_privilege < 5)
		return PrivilegeNames[m_privilege];
	return "Unknown";
}

PlayerPrivilege* PrivilegeSystem::GetPlayerPrivilege(int playerIndex)
{
	if (playerIndex < 0 || playerIndex > MAXPLAYERS)
		return nullptr;
	return &m_playerPrivileges[playerIndex];
}

void PrivilegeSystem::SetPlayerPrivilege(int playerIndex, PrivilegeLevel level, float expiryTime)
{
	PlayerPrivilege* pPrivilege = GetPlayerPrivilege(playerIndex);
	if (pPrivilege)
	{
		pPrivilege->SetPrivilege(level, expiryTime);
	}
}

void PrivilegeSystem::UpdatePrivileges()
{
	// Check for expired privileges
	for (int i = 1; i <= MAXPLAYERS; i++)
	{
		PlayerPrivilege* pPrivilege = &m_playerPrivileges[i];
		
		if (pPrivilege->GetPrivilege() != PRIVILEGE_USER && !pPrivilege->IsPrivilegeActive())
		{
			// Privilege expired, reset to user
			pPrivilege->SetPrivilege(PRIVILEGE_USER);
		}
	}
}
