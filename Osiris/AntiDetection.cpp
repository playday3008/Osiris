#include "AntiDetection.h"
#include <Windows.h>
#include <winternl.h>

HMODULE GetSelfModuleHandle()
{
	MEMORY_BASIC_INFORMATION mbi;

	return ::VirtualQuery(GetSelfModuleHandle, &mbi, sizeof mbi) != 0
		       ? static_cast<HMODULE>(mbi.AllocationBase) : nullptr;
}

void HideModule(void* pModule)
{
	auto pPEB = reinterpret_cast<PEB*>(__readfsdword(0x30));

	auto pLDR = pPEB->Ldr;
	auto pCurrent = pLDR->Reserved2[1];
	auto pNext = pCurrent;

	do
	{
		auto pNextPoint = *reinterpret_cast<void**>(static_cast<unsigned char*>(pNext));
		auto pLastPoint = *reinterpret_cast<void**>(static_cast<unsigned char*>(pNext) + 0x4);
		auto nBaseAddress = pPEB->Reserved3[1];

		if (nBaseAddress == pModule)
		{
			*reinterpret_cast<void**>(static_cast<unsigned char*>(pLastPoint)) = pNextPoint;
			*reinterpret_cast<void**>(static_cast<unsigned char*>(pNextPoint) + 0x4) = pLastPoint;
			pCurrent = pNextPoint;
		}

		pNext = *static_cast<void**>(pNext);
	} while (pCurrent != pNext);
}

AntiDetection::AntiDetection() {
	//clear up PE headers.
	const HMODULE hModule = GetSelfModuleHandle();
	DWORD dwMemPro;
	VirtualProtect(hModule, 0x1000, PAGE_EXECUTE_READWRITE, &dwMemPro);
	memset(hModule, 0, 0x1000);
	VirtualProtect(hModule, 0x1000, dwMemPro, &dwMemPro);
	if (IsDebuggerPresent())
		OutputDebugStringA("CleanUp PEheader Success.\n");
	HideModule(hModule);
	if (IsDebuggerPresent())
		OutputDebugStringA("Cutup PEB link success.\n");
}