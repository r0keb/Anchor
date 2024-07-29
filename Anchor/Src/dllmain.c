#include "HardBP.h"
#pragma warning(disable : 4996)


BOOL APIENTRY DllMain( HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved) {
    switch (ul_reason_for_call) {
        case DLL_PROCESS_ATTACH:

            ///
            /// debug console for the DLL
            /// 
            AllocConsole();

            freopen("CONOUT$", "w", stdout);
            freopen("CONOUT$", "w", stderr);
            freopen("CONIN$", "r", stdin);

            HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
            if (hConsole == INVALID_HANDLE_VALUE) {
                MessageBoxW(NULL, L"Error getting the handle to the console", L"ERROR", MB_OK | MB_ICONERROR);
                return;
            }

            printf("\n[+] Process ID -> %d", (GetProcessId((HANDLE)-1)));

            if (InitHardBp() != TRUE) {
                printf("\n[!] Error initializing the Hardware Breakpoint - %d\n", GetLastError());
	            return -1;
            }

            ///
            /// three different functions to hook (there are millions of functions, but I chose these three for the POC)
            /// 


            // NtTerminateProcess\
            PVOID pNtTerminateProcess = GetProcAddress(GetModuleHandle(L"ntdll.dll"), "NtTerminateProcess");\
            if (pNtTerminateProcess == NULL) {\
                printf("\n[!] ERROR getting the address of the function\n");\
                return -1;\
            }\
            if (SetHardBp(pNtTerminateProcess, BounceFunc, Dr2) != TRUE) {\
                printf("\n[MAIN] Error setting the hardware breakpoint\n");\
                return -1;\
            }\
            printf("\n[+] Hardware breakpoint setted at 0x%p\n", pNtTerminateProcess);\


            // NtAllocateVirtualMemory
            PVOID pNtAllocateVirtualMemory = GetProcAddress(GetModuleHandle(L"ntdll.dll"), "NtAllocateVirtualMemory");
            if (pNtAllocateVirtualMemory == NULL) {
                printf("\n[!] ERROR getting the address of the function\n");
                return -1;
            }
            if (SetHardBp(pNtAllocateVirtualMemory, BounceFunc, Dr2) != TRUE) {
	            printf("\n[MAIN] Error setting the hardware breakpoint\n");
	            return -1;
            }
            printf("\n[+] Hardware breakpoint setted at 0x%p\n", pNtAllocateVirtualMemory);


            // NtClose\
            PVOID pNtClose = GetProcAddress(GetModuleHandle(L"ntdll.dll"), "NtClose");\
            if (pNtClose == NULL) {\
                printf("\n[!] ERROR getting the address of the function\n");\
                return -1;\
            }\
            if (SetHardBp(pNtClose, BounceFunc, Dr2) != TRUE) {\
                printf("\n[MAIN] Error setting the hardware breakpoint\n");\
                return -1;\
            }\
            printf("\n[+] Hardware breakpoint setted at 0x%p\n", pNtClose);


        case DLL_THREAD_ATTACH:
        case DLL_THREAD_DETACH:
        case DLL_PROCESS_DETACH:
        
            break;
    }
    return TRUE;
}

