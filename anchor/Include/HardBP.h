#pragma once

#include <stdio.h>
#include <Windows.h>
#include <tlhelp32.h>

typedef enum _DRreg {
	Dr0,
	Dr1,
	Dr2,
	Dr3

}DRreg, * PDRreg;

/// 
/// Hardware Breakpoint hooking functions
/// 

BOOL InitHardBp();
BOOL Cleanup();
BOOL SetHardBp(IN PVOID pAddress, IN PVOID fnHookFunc, IN enum DRreg tempDRreg);
BOOL RemHardBp(IN enum DRreg tempDRreg);
LONG WINAPI VEHfunction(PEXCEPTION_POINTERS pExceptionInfo);
VOID BounceFunc(CONTEXT* ctx);
VOID RetFunc(CONTEXT * ctx);


/// 
/// Thread enum function
/// 

BOOL EnumThreads(IN DWORD dwPid, IN HANDLE hProcess, OUT HANDLE* hThread);
