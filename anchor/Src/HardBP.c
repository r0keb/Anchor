#include "HardBP.h"

// insert a "ret" instruction into the text section (executable code) so we can call it (opcode -> 0xc3)
#pragma section(".text")
__declspec(allocate(".text")) const BYTE bRet = 0xc3;


// Global flags
BOOL GlobalBounce = FALSE;
DWORD GlobaldwThreadId = NULL;


// exception handler to manage the EXCEPTION_SINGLE_STEP exception
PVOID VEHexceptionHandler = NULL;

// Critical section to avoid race condition
CRITICAL_SECTION CritSection = { 0 };
BOOL CritSectionInit = FALSE; // flag to make sure the critical section is initialized

// one bounce function per register (dr0-dr3)
PVOID pBounceFunction[4] = { 0 };


VOID RetFunc(CONTEXT* ctx) {
	ctx->Rip = (DWORD64)&bRet;
}

VOID MessageBoxFunc() {
	MessageBoxW(NULL, L"Hardware Breakpoint hook executed!", L"Anchor", MB_OK | MB_ICONEXCLAMATION);
}

// "Malicious" code to execute when the hardware bp is triggered
VOID BounceFunc(CONTEXT* ctx) {

	// flag to avoid the loop of the malicious code being executed
	if (GlobalBounce == FALSE) {

		// message box spawn
		// instead of executing MessageBoxW directly, we create a thread. Why?
		// to avoid interrupting the main execution flow of the main thread
		HANDLE hWindow = CreateThread(NULL, NULL, MessageBoxFunc, NULL, NULL, NULL);
		if (hWindow == NULL) {
			printf("\n[!] Error creating the message box - %d\n", GetLastError());
			return -1;
		}
		CloseHandle(hWindow);
		
		GlobalBounce = TRUE;
	}
	
	// Ret if we don't want to execute the original function after the custom code
	// RetFunc(&ctx);

}


// Initialize the exception function and critical section
BOOL InitHardBp() {

	if (CritSectionInit == FALSE) {
		InitializeCriticalSection(&CritSection);
		CritSectionInit = TRUE;
	}

	if (VEHexceptionHandler == NULL) {
		if ((VEHexceptionHandler = AddVectoredExceptionHandler(1, (PVECTORED_EXCEPTION_HANDLER)&VEHfunction)) == NULL) {
			printf("\n[!] Error setting the VEH exception handler - %d\n", GetLastError());
			return FALSE;
		}
	}

	return TRUE;
}


// clean all the hardware breakpoint registers and masks
BOOL Cleanup() {

	EnterCriticalSection(&CritSection);

	for (int i = 0; i < 4; i++) {
		RemHardBp(i);
	}
	
	LeaveCriticalSection(&CritSection);

	return TRUE;
}


// set the hardware breakpoint
BOOL SetHardBp(IN PVOID pAddress, IN PVOID fnHookFunc, IN enum DRreg tempDRreg) {

	// check
	if (fnHookFunc == NULL || pAddress == NULL || VEHfunction == NULL) {
		printf("\n[!] ERROR\n");
		return FALSE;
	}

	CONTEXT ctx = { .ContextFlags = CONTEXT_ALL };

	HANDLE hThread = NULL;

	if (GlobaldwThreadId == NULL) {
		// enumerate all the thread to get the handle to the main thread
		EnumThreads((GetProcessId((HANDLE)-1)), ((HANDLE)-1), &hThread);
		if (hThread == INVALID_HANDLE_VALUE) {
			printf("\n[!] Error getting the handle to the main thread - %d\n", GetLastError());
			return FALSE;
		}

		// we set the thread id to ensure we change correctly the context of the same threads in both functions
		GlobaldwThreadId = GetThreadId(hThread);
	}
	else {
		hThread = OpenThread(THREAD_ALL_ACCESS, FALSE, GlobaldwThreadId);
	}

	// get the thread context of the common thread
	if (GetThreadContext(hThread, &ctx) != TRUE) {
		printf("\n[!] Error getting the context\n");
		CloseHandle(hThread);
		return FALSE;
	}

	// set the hardware bp registers (dr0-dr3) and modify the debug control register (dr7) according to the previous
	// dr0-dr3: we only have to set the function address we want to set the bp (hook)
	// dr7: we are setting the local flag to turn on the register. also we have to set these fields: "Breakpoint Size" & "Breakpoint Trigger"
	switch (tempDRreg) {

	case Dr0:

		if (ctx.Dr0 == NULL) {
			ctx.Dr0 = pAddress;
			ctx.Dr7 |= (1UL << (tempDRreg * 2));
		}
		break;

	case Dr1:

		if (ctx.Dr1 == NULL) {
			ctx.Dr1 = pAddress;
			ctx.Dr7 |= (1UL << (tempDRreg * 2));
		}
		break;

	case Dr2:

		if (ctx.Dr2 == NULL) {
			ctx.Dr2 = pAddress;
			ctx.Dr7 |= (1UL << (tempDRreg * 2));
		}
		break;

	case Dr3:

		if (ctx.Dr3 == NULL) {
			ctx.Dr3 = pAddress;
			ctx.Dr7 |= (1UL << (tempDRreg * 2));
		}
		break;

	default:
		return FALSE;
	}

	EnterCriticalSection(&CritSection);

	// set the hookfunc into the array
	pBounceFunction[tempDRreg] = fnHookFunc;

	LeaveCriticalSection(&CritSection);

	// apply the modified context to the thread
	if (SetThreadContext(hThread, &ctx) != TRUE) {
		printf("\n[!] Error setting the context\n");
		CloseHandle(hThread); 
		hThread == NULL;
		return FALSE;
	}

	if (hThread) {
		CloseHandle(hThread);
	}

	return TRUE;
}


// remove the hardware breakpoint
BOOL RemHardBp(IN enum DRreg tempDRreg) {

	// check if the exception function is initialized
	if (VEHexceptionHandler == NULL) {
		printf("\nVEHexceptionHandler not initializated\n");
	}

	CONTEXT ctx = { .ContextFlags = CONTEXT_ALL };

	// get the handle to the thread we used in `SetHardBp`
	HANDLE hThread = OpenThread(THREAD_ALL_ACCESS, FALSE, GlobaldwThreadId);
	if (hThread == INVALID_HANDLE_VALUE) {
		printf("\n[!] Error getting the handle to the main thread - %d\n", GetLastError());
		return FALSE;
	}

	if (GetThreadContext(hThread, &ctx) != TRUE) {
		printf("\n[!] Error getting the context\n");
		CloseHandle(hThread);
		return FALSE;
	}

	// clear the address from the hardware bp registers (dr0-dr3) and reset the debug control register (dr7)
	switch (tempDRreg) {

	case Dr0:
		ctx.Dr0 = 0x00;
		ctx.Dr7 = 0x00;
		break;

	case Dr1:
		ctx.Dr1 = 0x00;
		ctx.Dr7 = 0x00;
		break;

	case Dr2:
		ctx.Dr2 = 0x00;
		ctx.Dr7 = 0x00;
		break;

	case Dr3:
		ctx.Dr3 = 0x00;
		ctx.Dr7 = 0x00;
		break;

	default:
		printf("\nnon hardware bp register founded\n");
	}

	if (SetThreadContext(hThread, &ctx) != TRUE) {
		printf("\n[!] Error getting the context\n");
		CloseHandle(hThread);
		return FALSE;
	}

	if (hThread) {
		CloseHandle(hThread);
	}

	return TRUE;
}


// exception function
LONG WINAPI VEHfunction(PEXCEPTION_POINTERS pExceptionInfo) {

	// check if the exception is a EXCEPTION_SINGLE_STEP (0x80000004)
	if (pExceptionInfo->ExceptionRecord->ExceptionCode == EXCEPTION_SINGLE_STEP) {

		// verify if the exception is our hardware bp
		if (pExceptionInfo->ContextRecord->Dr0 == pExceptionInfo->ExceptionRecord->ExceptionAddress ||
			pExceptionInfo->ContextRecord->Dr1 == pExceptionInfo->ExceptionRecord->ExceptionAddress ||
			pExceptionInfo->ContextRecord->Dr2 == pExceptionInfo->ExceptionRecord->ExceptionAddress ||
			pExceptionInfo->ContextRecord->Dr3 == pExceptionInfo->ExceptionRecord->ExceptionAddress) {

			enum DRreg tempDRreg = -1;
			VOID(*fnHookFunc)(PCONTEXT) = NULL;

			EnterCriticalSection(&CritSection);

			// check the EXCEPTION_POINTERS struct to ensure the exceptionAddress matches the hardware bp register address
			// if so, update the `tempDRreg` var to proceed with the execution
			if (pExceptionInfo->ContextRecord->Dr0 == pExceptionInfo->ExceptionRecord->ExceptionAddress) {
				tempDRreg = Dr0;
			}
			if (pExceptionInfo->ContextRecord->Dr1 == pExceptionInfo->ExceptionRecord->ExceptionAddress) {
				tempDRreg = Dr1;
			}
			if (pExceptionInfo->ContextRecord->Dr2 == pExceptionInfo->ExceptionRecord->ExceptionAddress) {
				tempDRreg = Dr2;
			}
			if (pExceptionInfo->ContextRecord->Dr3 == pExceptionInfo->ExceptionRecord->ExceptionAddress) {
				tempDRreg = Dr3;
			}

			// check if the tempDRreg is updated
			if (tempDRreg != -1) {
				// verify the global flag
				if (GlobalBounce != TRUE) {

					// temporarily remove the hardware breakpoint to avoid loops
					if (RemHardBp(tempDRreg) != TRUE) {
						printf("\n[!] Error removing the hardware breakpoint - %d\n", GetLastError());
						return FALSE;
					}

					// load the hook function address into the array
					fnHookFunc = pBounceFunction[tempDRreg];

					// call the function with the thread context as the argument
					fnHookFunc(pExceptionInfo->ContextRecord);

					// continue the execution by modifying the Resume flag bit (bit number 16) into the Eflags register
					pExceptionInfo->ContextRecord->EFlags = (pExceptionInfo->ContextRecord->EFlags | (1 << 16));

					// reset the hardware breakpoint. this implementation is set to execute the malicious code once, but it can be modified
					if (SetHardBp(pExceptionInfo->ExceptionRecord->ExceptionAddress, pBounceFunction[tempDRreg], tempDRreg) != TRUE) {
						printf("\n[!] Error setting again the hardware breakpoint - %d\n", GetLastError());
						return FALSE;
					}
					// debug flag to ensure the code is executed
					printf("\njuancked :u\n");
				}
				else {
					// if the bounce flag is set, no more code should execute
					pExceptionInfo->ContextRecord->EFlags = (pExceptionInfo->ContextRecord->EFlags | (1 << 16));
					Cleanup();
				}
				LeaveCriticalSection(&CritSection);
			}
		}
		return EXCEPTION_CONTINUE_EXECUTION;

	}

	return FALSE;
}