#pragma once
#include "dbg.h"
#include "interface.h"
#include "strtools.h"
#include "plat.h"

#ifdef _WIN32
#include <Psapi.h>
#endif

class CModule
{
public:
	CModule(const char *path, const char *module) :
		m_pszModule(module), m_pszPath(path)
	{
		char szModule[MAX_PATH];

		V_snprintf(szModule, MAX_PATH, "%s%s%s%s%s", Plat_GetGameDirectory(), path, MODULE_PREFIX, m_pszModule, MODULE_EXT);

		m_hModule = dlmount(szModule);

		if (!m_hModule)
			Error("Could not find %s\n", szModule);

#ifdef _WIN32
		MODULEINFO m_hModuleInfo;
		GetModuleInformation(GetCurrentProcess(), m_hModule, &m_hModuleInfo, sizeof(m_hModuleInfo));

		m_base = (void *)m_hModuleInfo.lpBaseOfDll;
		m_size = m_hModuleInfo.SizeOfImage;
#else
		if (int e = GetModuleInformation(szModule, &m_base, &m_size))
			Error("Failed to get module info for %s, error %d\n", szModule, e);
#endif
	}

	void *FindSignature(const byte *pData, size_t length)
	{
		if (!pData || length == 0 || !m_base || m_size == 0 || length > m_size)
			return nullptr;

		unsigned char *pMemory = (byte *)m_base;
		const size_t lastStart = m_size - length;

		for (size_t i = 0; i <= lastStart; i++)
		{
			bool matched = true;
			for (size_t j = 0; j < length; j++)
			{
				const byte needle = pData[j];
				if (needle != '\x2A' && pMemory[i + j] != needle)
				{
					matched = false;
					break;
				}
			}

			if (matched)
				return (void *)(pMemory + i);
		}

		return nullptr;
	}

	// Will break with string containing \0 !!!
	void *FindSignature(const byte *pData)
	{
		size_t iSigLength = V_strlen((const char *)pData);

		return FindSignature(pData, iSigLength);
	}


	const char *m_pszModule;
	const char* m_pszPath;
	HINSTANCE m_hModule;
	void* m_base;
	size_t m_size;
};