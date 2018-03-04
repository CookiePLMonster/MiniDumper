#include <windows.h>
#include <stdio.h>

#include <time.h>
#include <DbgHelp.h>

LONG WINAPI CustomUnhandledExceptionFilter(LPEXCEPTION_POINTERS ExceptionInfo)
{
        // step 1: write minidump
        char		error[1024];
		char		errorCode[1024];
        char		filename[MAX_PATH];
        __time64_t	time;
        struct tm	ltime;
		HANDLE		hFile;
		HWND		hWnd;

        _time64(&time);
        _localtime64_s(&ltime, &time);
        strftime(filename, sizeof(filename) - 1, "minidump-%Y%m%d%H%M%S.dmp", &ltime);
        sprintf_s(error, sizeof(error), "A minidump has been written to %s.", filename);

        //CreateDirectoryA("minidumps", NULL);
		hFile = CreateFileA(filename, GENERIC_WRITE, FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

        if (hFile != INVALID_HANDLE_VALUE)
        {
			MINIDUMP_EXCEPTION_INFORMATION ex;
                memset(&ex, 0, sizeof(ex));
                ex.ThreadId = GetCurrentThreadId();
                ex.ExceptionPointers = ExceptionInfo;
                ex.ClientPointers = TRUE;

                if (FAILED(MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), hFile, MiniDumpNormal, &ex, NULL, NULL)))
                        sprintf_s(error, sizeof(error), "An error (0x%X) occurred during writing %s.", GetLastError(), filename);

                CloseHandle(hFile);
        }
        else
			sprintf_s(error, sizeof(error), "An error (0x%X) occurred during creating %s.", GetLastError(), filename);

        // step 2: exit the application
		sprintf_s(errorCode, sizeof(errorCode), "Fatal error (0x%08X) at 0x%08X.\n%s", ExceptionInfo->ExceptionRecord->ExceptionCode, ExceptionInfo->ExceptionRecord->ExceptionAddress, error);
		ShowCursor(TRUE);
		hWnd = FindWindowA(0, "");
		SetForegroundWindow(hWnd);
		ShowCursor(TRUE);
		MessageBoxA(NULL, errorCode, "MiniDumper", MB_ICONERROR | MB_OK );
		hWnd = FindWindowA(0, "");
		SetForegroundWindow(hWnd);

		ExitProcess(0);
       // return 0;
}

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	if ( ul_reason_for_call == DLL_PROCESS_ATTACH )
	{
		AddVectoredExceptionHandler(1, CustomUnhandledExceptionFilter);
	}

	return TRUE;
}