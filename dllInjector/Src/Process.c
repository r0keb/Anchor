#include "common.h"

BOOL EnumProcesses(IN LPCWSTR pProcName, OUT DWORD* dwPid, OUT HANDLE *hProcess) {

	PROCESSENTRY32 pe = { .dwSize = sizeof(PROCESSENTRY32) };

	HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
	if (hSnapshot == INVALID_HANDLE_VALUE) {
		printf("\n[!] Error getting the process snapshot - %d\n", GetLastError());
		return FALSE;
	}

	if (Process32First(hSnapshot, &pe) != TRUE) {
		printf("\n[ERROR] Process32First - %d\n", GetLastError());
		CloseHandle(hSnapshot);
		return FALSE;
	}

	do {

		// wprintf(L"\n[PID - %d] %s\n", lppe.th32ProcessID, lppe.szExeFile);

		if (wcscmp(pe.szExeFile, pProcName) == 0) {
			wprintf(L"\n[PID - %d] %s", pe.th32ProcessID, pe.szExeFile);

			*dwPid = pe.th32ProcessID;
			*hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pe.th32ProcessID);
			CloseHandle(hSnapshot);
			if (*hProcess == INVALID_HANDLE_VALUE) {
				printf("\n[!] NULL Process handle - %d\n", GetLastError());
				return FALSE;
			}
			break;

		}

	} while (Process32Next(hSnapshot, &pe));

	return TRUE;
}

