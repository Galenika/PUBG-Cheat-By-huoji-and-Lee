// stdafx.h : 标准系统包含文件的包含文件，
// 或是经常使用但不常更改的
// 特定于项目的包含文件
//

#pragma once

#include "targetver.h"

#include <stdio.h>
#include <tchar.h>



// TODO:  在此处引用程序需要的其他头文件
#include <Windows.h>
#include <iostream>

#include <d3d9.h>
#include <d3dx9.h>
#pragma comment(lib, "d3d9.lib")
#pragma comment(lib, "d3dx9.lib")

#include <dwmapi.h>
#pragma comment(lib, "dwmapi.lib")
#include "Bypass.h"

#include "imgui\imgui.h"
#include "imgui\dx9\imgui_dx9.h"
#include "Vars.h"
#include "Memory.h"
//---Fuck IT
#define WIN32_LEAN_AND_MEAN
#define NO_STRICT
#define NOMINMAX
#include <ntstatus.h>
#define WIN32_NO_STATUS
//#include <WinSock2.h>
#include <Windows.h>
#include <winternl.h>
#include "psapi.h"
#pragma comment(lib, "psapi.lib")
#include <vector>

#include <Iphlpapi.h>
#include "MD5.h"
#include "AntiLeak.h"
#include <fstream>
using namespace std;
#pragma comment(lib,"Iphlpapi.lib") //需要添加Iphlpapi.lib库
/*
#ifndef _WIN64
#error "x64 only please"
#endif
*/
typedef struct _EPROCESS *PEPROCESS;
typedef struct _HANDLE_TABLE_ENTRY_INFO *PHANDLE_TABLE_ENTRY_INFO;
typedef PVOID EX_PUSH_LOCK;
typedef struct _HANDLE_TABLE* PHANDLE_TABLE;

typedef struct _HANDLE_TABLE_ENTRY
{
	//This struct is incomplete, but we dont really care about the other fields
	ULONGLONG Value;
	ULONGLONG GrantedAccess : 25;
} HANDLE_TABLE_ENTRY, *PHANDLE_TABLE_ENTRY;

typedef struct _HANDLE_TABLE
{
	CHAR fill[100];
} HANDLE_TABLE, *PHANDLE_TABLE;
typedef struct Vector3D {
	float x;
	float y;
	float z;
}Vector, Vector3D;
#include "scm.h"
#include "sup.h"
#include "process.hpp"
#include "cpuz_driver.hpp"
#include "dyn_data.hpp"
#include "D3D.h"
#include <math.h>
#include <stdio.h>

// NTQUERYSYSTEMINFORMATION
typedef struct _SYSTEM_HANDLE {
	DWORD dwProcessId;
	BYTE bObjectType;
	BYTE bFlags;
	WORD wValue;
	PVOID pAddress;
	DWORD GrantedAccess;
} SYSTEM_HANDLE, *PSYSTEM_HANDLE;
typedef struct _SYSTEM_HANDLE_INFORMATION {
	DWORD dwCount;
	SYSTEM_HANDLE Handles[1];
} SYSTEM_HANDLE_INFORMATION, *PSYSTEM_HANDLE_INFORMATION;

typedef NTSTATUS(WINAPI *NTQUERYSYSTEMINFORMATION)(
	IN SYSTEM_INFORMATION_CLASS SystemInformationClass,
	OUT PVOID SystemInformation,
	IN ULONG SystemInformationLength,
	OUT PULONG ReturnLength OPTIONAL);

typedef NTSTATUS(WINAPI *NtQueryInformationProcessEx)(
	IN HANDLE ProcessHandle,
	IN PROCESSINFOCLASS ProcessInformationClass,
	OUT PVOID ProcessInformation,
	IN ULONG ProcessInformationLength,
	OUT PULONG ReturnLength OPTIONAL);

typedef NTSTATUS (WINAPI *NTCLOSE)(
	_In_ HANDLE Handle
);
typedef VOID(WINAPI *RtlInitUnicodeStringEX)(
	_Inout_  PUNICODE_STRING DestinationString,
	_In_opt_ PCWSTR          SourceString
);
typedef NTSTATUS(WINAPI *NtCreateFileEx)(
	_Out_    PHANDLE            FileHandle,
	_In_     ACCESS_MASK        DesiredAccess,
	_In_     POBJECT_ATTRIBUTES ObjectAttributes,
	_Out_    PIO_STATUS_BLOCK   IoStatusBlock,
	_In_opt_ PLARGE_INTEGER     AllocationSize,
	_In_     ULONG              FileAttributes,
	_In_     ULONG              ShareAccess,
	_In_     ULONG              CreateDisposition,
	_In_     ULONG              CreateOptions,
	_In_     PVOID              EaBuffer,
	_In_     ULONG              EaLength
);
typedef NTSTATUS (WINAPI *NtOpenFileEx)(
	_Out_ PHANDLE            FileHandle,
	_In_  ACCESS_MASK        DesiredAccess,
	_In_  POBJECT_ATTRIBUTES ObjectAttributes,
	_Out_ PIO_STATUS_BLOCK   IoStatusBlock,
	_In_  ULONG              ShareAccess,
	_In_  ULONG              OpenOptions
);

typedef HRESULT (*FDirect3DCreate9Ex)(
	_In_  UINT         SDKVersion,
	_Out_ IDirect3D9Ex **ppD3D
);

typedef HRESULT (*D3DXCreateFontWEx)(
	LPDIRECT3DDEVICE9       pDevice,
	INT                     Height,
	UINT                    Width,
	UINT                    Weight,
	UINT                    MipLevels,
	BOOL                    Italic,
	DWORD                   CharSet,
	DWORD                   OutputPrecision,
	DWORD                   Quality,
	DWORD                   PitchAndFamily,
	LPCWSTR                 pFaceName,
	LPD3DXFONT*             ppFont);

typedef HRESULT (WINAPI *D3DXCreateLineEx)(
	LPDIRECT3DDEVICE9   pDevice,
	LPD3DXLINE*         ppLine);
	
static AntiLeak A;