#include "stdafx.h"

#define MAX_LOG 100

BOOL g_OneShot;

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
	LONG_PTR style;

	if (OBJID_WINDOW != idObject || EVENT_OBJECT_CREATE != event)
		return;

	if (!hwnd)
		return;

	i = GetWindowTextW(hwnd, buf, 3);
	if (1 != i)
		return;
	if (L'X' != buf[0])
		return;

	i = GetClassNameW(hwnd, buf, 3);
	if (1 != i)
		return;
	if (L'X' != buf[0])
		return;

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

	if (g_OneShot)
		PostQuitMessage(0);
}

STARTUPINFOW si = { 0 };

int Startup()
{
	PROCESS_INFORMATION pi;
	LPCWSTR cmdline;
	LPCWSTR* argv;
	int argc;
	HWINEVENTHOOK hHook = NULL;
	DWORD pidX, dw;
	MSG msg;
	int i, result;
	BOOL loop;
	BOOL b;
	DEBUG_EVENT dbg;

	cmdline = GetCommandLineW();
	Log(L"XFrameFix: cmdline = %1\n", cmdline);
	argc = 0;
	argv = CommandLineToArgvW(cmdline, &argc);
	g_OneShot = 1 < argc;
	for (i = 0; i < argc; ++i)
		Log(L"XFrameFix: argv[%1!d!] = %2\n", i, argv[i]);

	if (!g_OneShot)
	{
		if (!CreateMutexW(NULL, TRUE, L"XFrameFix single instance") || GetLastError() == ERROR_ALREADY_EXISTS)
			goto cleanup;
	}

	if (g_OneShot)
	{
		if (argc < 2)
			goto cleanup;

		Log(L"XFrameFix: creating X process\n");

		si.cb = sizeof(STARTUPINFOW);
		if (!CreateProcessW(
			argv[1],
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
			goto cleanup;
		}
		pidX = pi.dwProcessId;
	}
	else
	{
		pidX = 0;
	}

	Log(L"XFrameFix: setting hook\n");
	hHook = SetWinEventHook(EVENT_OBJECT_CREATE, EVENT_OBJECT_CREATE, GetModuleHandleW(NULL), (WINEVENTPROC)WinEventProc, pidX, 0, WINEVENT_OUTOFCONTEXT);
	if (!hHook)
		goto cleanup;

	if (g_OneShot)
	{
		b = WaitForDebugEvent(&dbg, INFINITE);
		if (!b)
		{
			Log(L"XFrameFix: WaitForDebugEvent failed with code %1!d!\n", GetLastError());
			b = TerminateProcess(pi.hProcess, 1);
			if (!b)
				Log(L"XFrameFix: TerminateProcess failed with code %1!d!\n", GetLastError());
			else
				Log(L"XFrameFix: terminated X process\n");
			goto cleanup;
		}
		else
		{
			Log(L"XFrameFix: got debug event %1!d!\n", dbg.dwDebugEventCode);
			switch (dbg.dwDebugEventCode)
			{
			case CREATE_PROCESS_DEBUG_EVENT:
				break;
			default:
				break;
			}

			b = DebugActiveProcessStop(pidX);
			if (!b)
			{
				Log(L"XFrameFix: DebugActiveProcessStop failed with code %1!d!\n", GetLastError());
				b = TerminateProcess(pi.hProcess, 1);
				if (!b)
					Log(L"XFrameFix: TerminateProcess failed with code %1!d!\n", GetLastError());
				else
					Log(L"XFrameFix: terminated X process\n");
				goto cleanup;
			}
		}

		loop = TRUE;
		while (loop)
		{
			dw = MsgWaitForMultipleObjects(1, &pi.hProcess, FALSE, INFINITE, QS_ALLINPUT);
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
	}
	else
	{

		while (GetMessageW(&msg, NULL, 0, 0))
		{
			TranslateMessage(&msg);
			DispatchMessageW(&msg);
		}

		result = msg.wParam;
	}

cleanup:
	if (hHook)
	{
		b = UnhookWinEvent(hHook);
		if (b)
			Log(L"XFrameFix: hook removed\n");
		else
			Log(L"XFrameFix: UnhookWinEvent failed with code %1!d!\n", GetLastError());
	}

	Log(L"XFrameFix: end\n");
	ExitProcess(result);
	return result;
}
