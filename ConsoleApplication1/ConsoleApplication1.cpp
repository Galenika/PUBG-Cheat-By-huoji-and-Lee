// ConsoleApplication1.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <ntstatus.h>
#include <Windows.h>
/*
void CheatThread()
{
	bool temp = false;
	if (FuckBE::GetHandle())
		temp = true;
	while (true)
	{
		if (GetAsyncKeyState(VK_F6)) break;
		Sleep(40);
	}
	Vars.Lazy.Flags = true;
	D::hInstance = NULL;
	//	Vars.Lazy.GCmdLine = nCmdLine;

	//	if (A.MainCheck())
	//		int a = 186 + 1222;
	//	else
	//		exit(1);
	Vars.Lazy.Flags = true;
	if (temp && Vars.Lazy.Flags)
	{
		Vars.Lazy.Flags = true;
		D::Start();
	}

	
}

DWORD WINAPI DllMain(HMODULE hDll, DWORD dwReason, LPVOID lpReserved)
{
	switch (dwReason)
	{
	case DLL_PROCESS_ATTACH:
	{
		MessageBox(0, L"exit!!!!", L"exit!!!!", MB_OK | MB_ICONERROR);
		//exit(1);
		CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)CheatThread, NULL, NULL, NULL);
		return TRUE;
	}
	case DLL_PROCESS_DETACH:
	{
		return TRUE;
	}
	return FALSE;
	}
}
*/
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hSecInstance, LPSTR nCmdLine, INT nCmdShow)
{

	time_t tt = time(NULL);
	tm* t = localtime(&tt);
	char buffer[128];
	sprintf_s(buffer, "%d-%02d-%02d%02d:%02d",
		t->tm_year + 1900,
		t->tm_mon + 1,
		t->tm_mday,
		t->tm_hour,
		t->tm_min);

	string aTime(buffer);
	Vars.Lazy.Time = aTime;
	
	// 创建互斥量  
	HANDLE m_hMutex = CreateMutex(NULL, FALSE, L"YourAreMemeCreakerSSS");
	DWORD dwRet = GetLastError();
	if (m_hMutex)
	{
		if (ERROR_ALREADY_EXISTS == dwRet)
		{
			CloseHandle(m_hMutex);
			return 0;
			exit(0);
		}
	}
	else
	{
		CloseHandle(m_hMutex);
		return 0;
		exit(0);
	}
	bool temp = false;
	if (FuckBE::GetHandle())
		temp = true;

	while (true)
	{
		if (GetAsyncKeyState(VK_F6)) break;
		Sleep(40);
	}
	Vars.Lazy.Flags = false;

	D::hInstance = hInstance;
	Vars.Lazy.GCmdLine = nCmdLine;

//	if (A.MainCheck())
	//	int a = 186 + 1222;
	//else
	//	exit(1);
	Vars.Lazy.Flags = true;
	if (temp && Vars.Lazy.Flags)
	{
		D::Start();
	}
		

	return 0;
}



