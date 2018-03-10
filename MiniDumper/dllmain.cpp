#include <windows.h>
#include <stdio.h>

#include <time.h>
#include <DbgHelp.h>
#include "MemoryMgr.h"

#define INCLUDE_MESSAGEBOX 0

LONG WINAPI CustomUnhandledExceptionFilter(LPEXCEPTION_POINTERS ExceptionInfo)
{
        // step 1: write minidump
#if INCLUDE_MESSAGEBOX
        wchar_t		error[1024];
		wchar_t		errorCode[1024];
#endif
		wchar_t		modulename[MAX_PATH];
		wchar_t		filename[MAX_PATH];
		wchar_t		timestamp[128];
        __time64_t	time;
        struct tm	ltime;
		HANDLE		hFile;
		HWND		hWnd;

		const wchar_t* modulenameptr;
		if ( GetModuleFileNameW( GetModuleHandle(NULL), modulename, _countof(modulename)) != 0 )
		{
			modulenameptr = wcsrchr(modulename, '\\') + 1;
		}
		else
		{
			modulenameptr = L"err.err";
		}

        _time64(&time);
        _localtime64_s(&ltime, &time);
        wcsftime(timestamp, _countof(timestamp), L"%Y%m%d%H%M%S", &ltime);
		swprintf_s(filename, L"%s.%s.dmp", modulenameptr, timestamp);
#if INCLUDE_MESSAGEBOX
		swprintf_s(error, L"A minidump has been written to %s.", filename);
#endif

        //CreateDirectoryA("minidumps", NULL);
		hFile = CreateFileW(filename, GENERIC_WRITE, FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

        if (hFile != INVALID_HANDLE_VALUE)
        {
			MINIDUMP_EXCEPTION_INFORMATION ex;
                memset(&ex, 0, sizeof(ex));
                ex.ThreadId = GetCurrentThreadId();
                ex.ExceptionPointers = ExceptionInfo;
                ex.ClientPointers = TRUE;

                if (FAILED(MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), hFile, MiniDumpWithDataSegs, &ex, NULL, NULL)))
				{
#if INCLUDE_MESSAGEBOX
					swprintf_s(error, L"An error (0x%X) occurred during writing %s.", GetLastError(), filename);
#endif
				}

                CloseHandle(hFile);
        }
        else
		{
#if INCLUDE_MESSAGEBOX
			swprintf_s(error, L"An error (0x%X) occurred during creating %s.", GetLastError(), filename);
#endif
		}

        // step 2: exit the application
#if INCLUDE_MESSAGEBOX
		swprintf_s(errorCode, L"Fatal error (0x%08X) at 0x%08X.\n%s", ExceptionInfo->ExceptionRecord->ExceptionCode, ExceptionInfo->ExceptionRecord->ExceptionAddress, error);
#endif
		ShowCursor(TRUE);
		hWnd = FindWindowW(0, L"");
		SetForegroundWindow(hWnd);
#if INCLUDE_MESSAGEBOX
		ShowCursor(TRUE);
		MessageBoxW(NULL, errorCode, L"MiniDumper", MB_ICONERROR | MB_OK );
		hWnd = FindWindowW(0, L"");
		SetForegroundWindow(hWnd);
#endif

		return EXCEPTION_CONTINUE_SEARCH;
}

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	if ( ul_reason_for_call == DLL_PROCESS_ATTACH )
	{
		SetUnhandledExceptionFilter(CustomUnhandledExceptionFilter);

		// Now stub out CustomUnhandledExceptionFilter so NO ONE ELSE can set it!
		Memory::VP::Patch( &SetUnhandledExceptionFilter, { 0xC2, 0x04, 0x00 } );
	}

	return TRUE;
}