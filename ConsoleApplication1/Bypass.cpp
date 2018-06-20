#include "stdafx.h"
#include "scm.h"
#include "sup.h"
#include "process.hpp"
#include "cpuz_driver.hpp"
#include "dyn_data.hpp"
#include <TlHelp32.h>
HANDLE handle;

using namespace std;
#pragma warning(disable: 4996)


bool FuckBE::GetHandle()
{
	dyn_data::load_information();
	uint32_t pid;
	while (1) {
		const wchar_t TslGame[12] = { 0x54,0x73,0x6c,0x47,0x61,0x6d,0x65,0x2e,0x65,0x78,0x65 };
		pid = process::find(TslGame);
		if (pid)
			break;
	}
	handle = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pid);
	if (!handle)
		return false;
	if (process::attach(GetCurrentProcessId())) {
		if (!process::grant_handle_access(handle, PROCESS_ALL_ACCESS))
			return false;
		process::detach();
	}
	D::Pid = pid;	
	
	return true;
}