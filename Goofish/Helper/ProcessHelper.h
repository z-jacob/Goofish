#pragma once
#include <windows.h>

void KillSelfProcess()
{
	HANDLE hProcess = GetCurrentProcess();
	TerminateProcess(hProcess, 0);
	CloseHandle(hProcess);
}