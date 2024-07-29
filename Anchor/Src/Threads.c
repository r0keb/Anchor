#include "HardBP.h"

// thread enum function
BOOL EnumThreads(IN DWORD dwPid, IN HANDLE hProcess, OUT HANDLE* hThread) {

	THREADENTRY32 te = { .dwSize = sizeof(THREADENTRY32) };
	HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, dwPid);
	if (hSnapshot == INVALID_HANDLE_VALUE) {
		printf("\n[!] Error getting the process snapshot - %d\n", GetLastError());
		return FALSE;
	}

	if (Thread32First(hSnapshot, &te) != TRUE) {
		printf("\n[ERROR] Thread32First - %d\n", GetLastError());
		CloseHandle(hSnapshot);
		return FALSE;
	}

	do {

		if (te.th32OwnerProcessID == dwPid) {
			wprintf(L"\n[+] Thread ID of the main thread -> %d", te.th32ThreadID);
			*hThread = OpenThread(THREAD_ALL_ACCESS, FALSE, te.th32ThreadID);
			CloseHandle(hSnapshot);
			break;
		}

	} while (Thread32Next(hSnapshot, &te));

	return TRUE;
}