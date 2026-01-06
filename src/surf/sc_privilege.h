#pragma once

#include "common.h"

enum PrivilegeLevel
{
	PRIVILEGE_USER = 0,
	PRIVILEGE_DONOR = 1,
	PRIVILEGE_VIP = 2,
	PRIVILEGE_ADMIN = 3,
	PRIVILEGE_OWNER = 4
};

class PlayerPrivilege
{
public:
	PlayerPrivilege() : m_privilege(PRIVILEGE_USER), m_expiryTime(0.0f) {}

	void SetPrivilege(PrivilegeLevel level, float expiryTime = -1.0f)
	{
		m_privilege = level;
		m_expiryTime = expiryTime; // -1 for permanent, 0 for no privilege, or timestamp for expiry
	}

	PrivilegeLevel GetPrivilege() const { return m_privilege; }
	float GetExpiryTime() const { return m_expiryTime; }
	bool IsPrivilegeActive() const;
	const char* GetPrivilegeName() const;

private:
	PrivilegeLevel m_privilege;
	float m_expiryTime;
};

class PrivilegeSystem
{
public:
	PlayerPrivilege* GetPlayerPrivilege(int playerIndex);
	void SetPlayerPrivilege(int playerIndex, PrivilegeLevel level, float expiryTime = -1.0f);
	void UpdatePrivileges();

private:
	PlayerPrivilege m_playerPrivileges[MAXPLAYERS + 1];
};

extern PrivilegeSystem g_PrivilegeSystem;
