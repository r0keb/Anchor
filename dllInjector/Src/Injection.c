#include "common.h"

BOOL DllInjection(HANDLE hProcess, LPCWSTR pDllName) {

	SIZE_T dwDllName = lstrlenW(pDllName) * sizeof(WCHAR);
	SIZE_T lpNumberOfBytesWritten = NULL;

	PVOID pLoadLibraryW = GetProcAddress(GetModuleHandle(L"kernel32.dll"), "LoadLibraryW");
	if (pLoadLibraryW == NULL) {
		printf("\n[!] error getting the address to the function - %d \n", GetLastError());
		return FALSE;
	}

	PVOID pAddress = VirtualAllocEx(hProcess, NULL, dwDllName, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
	if (pAddress == NULL) {
		printf("\n[!] error allocating memory - %d \n", GetLastError());
		return FALSE;
	}

	printf("\n[+] Dll name located at 0x%p", pAddress);

	if (!WriteProcessMemory(hProcess, pAddress, pDllName, dwDllName, &lpNumberOfBytesWritten) || lpNumberOfBytesWritten != dwDllName) {
		printf("\n[!] error writting the dllname into the remote process - %d \n", GetLastError());
		return FALSE;
	}

	printf("\n[+] %d bytes written", lpNumberOfBytesWritten);

	printf("\n[+] loading the dll into the remote process");
	HANDLE hThread = CreateRemoteThread(hProcess, NULL, NULL, pLoadLibraryW, pAddress, NULL, NULL);
	if (hThread == INVALID_HANDLE_VALUE) {
		printf("\n[!] Error creating the remote thread - %d \n", GetLastError());
		return FALSE;
	}
	printf("\nbien :u\n");

	WaitForSingleObject(hThread, INFINITE);

	VirtualFreeEx(hProcess, dwDllName, 0, MEM_RELEASE);
	CloseHandle(hThread);
	CloseHandle(hProcess);

	return TRUE;
}