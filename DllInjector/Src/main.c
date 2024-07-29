#include "common.h"

int wmain(int argc, wchar_t *argv[]) {

	DWORD dwPID = NULL;
	HANDLE hProcess = NULL;
	HANDLE hThread = NULL;

	if (argc < 3) {
		printf("\n[!] USAGE: \"program.exe <dll name> <process name>\"\n");
		printf("\t[*] DISCLAIMER: <process name> reffers to the running process, not the image\n");
		return -1;
	}

	LPCWSTR pProcName = argv[2];
	LPCWSTR pDllName = argv[1];


	if (EnumProcesses(pProcName, &dwPID, &hProcess) != TRUE) {
		return -1;
	}

	if (DllInjection(hProcess, pDllName) != TRUE) {
		return -1;
	}

	return 0;
}