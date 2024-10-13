#include "memory.h"

memory::memory(std::string_view procN)
{
	PROCESSENTRY32 entry{};
	entry.dwSize = sizeof(PROCESSENTRY32);

	HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

	if (Process32First(snapshot, &entry))
	{
		if (!procN.compare(entry.szExeFile))
		{
			procID = entry.th32ProcessID;
			procH = OpenProcess(PROCESS_ALL_ACCESS, FALSE, procID);
		}
		else
		{
			while (Process32Next(snapshot, &entry))
			{
				if (!procN.compare(entry.szExeFile))
				{
					procID = entry.th32ProcessID;
					procH = OpenProcess(PROCESS_ALL_ACCESS, FALSE, procID);
					break;
				}
			}
		}
		
	}
	if (snapshot)
	{
		CloseHandle(snapshot);
	}
}

std::uintptr_t memory::GetModuleBaseAddr(std::string_view moduleN) const
{
	MODULEENTRY32 entry{};
	entry.dwSize = sizeof(MODULEENTRY32);

	HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, procID);

	std::uintptr_t result{};

	if (Module32First(snapshot, &entry))
	{
		if (!moduleN.compare(entry.szModule))
		{
			result = reinterpret_cast<std::uintptr_t>(entry.modBaseAddr);
		}
		else
		{
			while (Module32Next(snapshot, &entry))
			{
				if (!moduleN.compare(entry.szModule))
				{
					result = reinterpret_cast<std::uintptr_t>(entry.modBaseAddr);
					break;
				}
			}
		}

	}
	if (snapshot)
	{
		CloseHandle(snapshot);
	}
	return result;
}