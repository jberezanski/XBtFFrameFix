#include "stdafx.h"

#define MAX_LOG 100

BOOL g_SingleProcess;

void Log(LPCWSTR fmt, ...)
{
	WCHAR buf[MAX_LOG];
	DWORD dw;

	va_list args;
	va_start(args, fmt);

	dw = FormatMessageW(FORMAT_MESSAGE_FROM_STRING, fmt, 0, 0, buf, MAX_LOG, &args);
	if (dw)
		OutputDebugStringW(buf);

	va_end(args);
}

void RemoveWindowFrame(HWND hwnd)
{
	LONG_PTR style;

	Log(L"XFrameFix: X window found: 0x%1!X!\n", hwnd);

	style = GetWindowLongPtrW(hwnd, GWL_STYLE);
	if (!style)
		return;

	if (!(style & (WS_DLGFRAME | WS_CAPTION)))
		return;

	style &= ~(WS_DLGFRAME | WS_CAPTION);
	if (SetWindowLongPtrW(hwnd, GWL_STYLE, style))
		SetWindowPos(hwnd, NULL, 0, 0, 0, 0, SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER);

	Log(L"XFrameFix: X window fixed\n");
}

void CALLBACK WinEventProc(
	HWINEVENTHOOK hWinEventHook,
	DWORD         event,
	HWND          hwnd,
	LONG          idObject,
	LONG          idChild,
	DWORD         idEventThread,
	DWORD         dwmsEventTime)
{
	WCHAR buf[3];
	int i;

	if (OBJID_WINDOW != idObject || EVENT_OBJECT_CREATE != event)
		return;

	if (!hwnd)
		return;

	i = GetClassNameW(hwnd, buf, 3);
	if (1 != i)
		return;
	if (L'X' != buf[0])
		return;

	i = GetWindowTextW(hwnd, buf, 3);
	if (1 != i)
		return;
	if (L'X' != buf[0])
		return;

	RemoveWindowFrame(hwnd);

	if (g_SingleProcess)
		PostQuitMessage(0);
}

BOOL CreateObjectCreateWinEventHook(DWORD pidTarget)
{
	HWINEVENTHOOK hHook = NULL;
	Log(L"XFrameFix: setting hook\n");
	hHook = SetWinEventHook(EVENT_OBJECT_CREATE, EVENT_OBJECT_CREATE, GetModuleHandleW(NULL), (WINEVENTPROC)WinEventProc, pidTarget, 0, WINEVENT_OUTOFCONTEXT);
	return !!hHook;
}

int RunSimpleMessageLoop()
{
	MSG msg;

	while (GetMessageW(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessageW(&msg);
	}

	return msg.wParam;
}

BOOL CreateTargetProcess(LPCWSTR szCmdLine, LPDWORD lpPid, LPHANDLE lpProcessHandle)
{
	PROCESS_INFORMATION pi;
	STARTUPINFOW si = { 0 };

	*lpPid = 0;
	*lpProcessHandle = INVALID_HANDLE_VALUE;

	Log(L"XFrameFix: creating X process\n");

	si.cb = sizeof(STARTUPINFOW);
	if (!CreateProcessW(
		szCmdLine,
		NULL,
		NULL,
		NULL,
		FALSE,
		DEBUG_PROCESS | DEBUG_ONLY_THIS_PROCESS,
		NULL,
		NULL,
		&si,
		&pi))
	{
		Log(L"XFrameFix: CreateProcess failed with code %1!d!\n", GetLastError());
		return FALSE;
	}

	*lpPid = pi.dwProcessId;
	*lpProcessHandle = pi.hProcess;
	return TRUE;
}

void KillTargetProcess(HANDLE hProcess)
{
	BOOL b;

	b = TerminateProcess(hProcess, 1);
	if (!b)
		Log(L"XFrameFix: TerminateProcess failed with code %1!d!\n", GetLastError());
	else
		Log(L"XFrameFix: terminated X process\n");
}

BOOL StopDebuggingAndResumeTargetProcess(DWORD pid)
{
	DEBUG_EVENT dbg;
	BOOL b;

	b = WaitForDebugEvent(&dbg, INFINITE);
	if (!b)
	{
		Log(L"XFrameFix: WaitForDebugEvent failed with code %1!d!\n", GetLastError());
		return FALSE;
	}
		
	Log(L"XFrameFix: got debug event %1!d!\n", dbg.dwDebugEventCode);
	switch (dbg.dwDebugEventCode)
	{
	case CREATE_PROCESS_DEBUG_EVENT:
		break;
	default:
		break;
	}

	b = DebugActiveProcessStop(pid);
	if (!b)
	{
		Log(L"XFrameFix: DebugActiveProcessStop failed with code %1!d!\n", GetLastError());
		return FALSE;
	}

	return TRUE;
}

int RunMessageLoopWithHandleWait(HANDLE hProcess)
{
	MSG msg;
	DWORD dw;
	int result;
	BOOL loop = TRUE;

	while (loop)
	{
		dw = MsgWaitForMultipleObjects(1, &hProcess, FALSE, INFINITE, QS_ALLINPUT);
		switch (dw)
		{
			case WAIT_OBJECT_0:
				Log(L"XFrameFix: X process ended\n");
				PostQuitMessage(0);
				loop = FALSE;
				result = 0;
				break;
			case WAIT_OBJECT_0 + 1:
				while (PeekMessageW(&msg, NULL, 0, 0, PM_REMOVE)) 
				{
					if (msg.message == WM_QUIT)
					{
						Log(L"XFrameFix: got WM_QUIT\n");
						loop = FALSE;
						result = msg.wParam;
						break;
					}
					else
					{
						TranslateMessage(&msg);
						DispatchMessageW(&msg);
					}
				}
				break;
			default:
				Log(L"XFrameFix: unexpected failure!\n");
				loop = FALSE;
				result = 1;
				break;
		}
	}

	return result;
}

int RunSingleProcess(LPCWSTR szCmdLine)
{
	DWORD pid;
	HANDLE hProcess;

	if (!CreateTargetProcess(szCmdLine, &pid, &hProcess))
		return 1;

	if (!CreateObjectCreateWinEventHook(pid))
	{
		KillTargetProcess(hProcess);
		return 1;
	}

	if (!StopDebuggingAndResumeTargetProcess(pid))
	{
		KillTargetProcess(hProcess);
		return 1;
	}

	return RunMessageLoopWithHandleWait(hProcess);
}

BOOL CreateSingleInstanceMutex()
{
	return CreateMutexW(NULL, TRUE, L"XFrameFix single instance") && GetLastError() != ERROR_ALREADY_EXISTS;
}

BOOL FindExistingWindow(HWND* lpHwnd)
{
	*lpHwnd = FindWindowW(L"X", L"X");
	return !!(*lpHwnd);
}

int RunContinously()
{
	HWND hwnd;

	if (!g_SingleProcess && !CreateSingleInstanceMutex())
		return 1;

	if (FindExistingWindow(&hwnd))
		RemoveWindowFrame(hwnd);

	if (!CreateObjectCreateWinEventHook(0))
		return 1;

	return RunSimpleMessageLoop();
}

BOOL CheckIfSingleProcess(LPCWSTR* ppszTargetProcessCmdLine)
{
	LPCWSTR cmdline;
	LPCWSTR* argv;
	int argc;
	int i;
	BOOL singleProcess;

	*ppszTargetProcessCmdLine = NULL;

	cmdline = GetCommandLineW();
	Log(L"XFrameFix: cmdline = %1\n", cmdline);
	argc = 0;
	argv = CommandLineToArgvW(cmdline, &argc);
	singleProcess = 1 < argc;
	for (i = 0; i < argc; ++i)
		Log(L"XFrameFix: argv[%1!d!] = %2\n", i, argv[i]);

	if (singleProcess)
		*ppszTargetProcessCmdLine = argv[1];

	return singleProcess;
}

void __cdecl Startup()
{
	int result;
	LPCWSTR szTargetCmdLine;
	
	g_SingleProcess = CheckIfSingleProcess(&szTargetCmdLine);

	if (g_SingleProcess)
		result = RunSingleProcess(szTargetCmdLine);
	else
		result = RunContinously();

	ExitProcess(result);
}
