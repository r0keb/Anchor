#pragma once

#include <stdio.h>
#include <Windows.h>
#include <tlhelp32.h>

BOOL DllInjection(HANDLE hProcess, LPCWSTR pDllName);
BOOL EnumProcesses(IN LPCWSTR pProcName, OUT DWORD* dwPid, OUT HANDLE* hProcess);
