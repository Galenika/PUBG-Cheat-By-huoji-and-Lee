#include "stdafx.h"
#include "Memory.h"
#include "D3D.h"

QWORD BaseAddress;
QWORD SizeOfImage;
QWORD ULocalPlayer;
QWORD ViewPortClient;
QWORD LocalPlayer;
QWORD ULevel;
QWORD PlayerController;
QWORD EntityList;
QWORD UWorld;
QWORD GameInstance;
QWORD GNames;
QWORD Pword;
QWORD ModuleHandle;
QWORD LocalrootComponent;
QWORD LocalPlayerController;
D3DXVECTOR3 Myangles;
QWORD CameraManager;
D3DXVECTOR3 LocalPos;
FCameraCacheEntry CameraCache;
int EntityCount;

bool SelectModule(LPCVOID hModule)
{
	ModuleHandle = (QWORD)hModule;
	LPVOID g_arrModuleBuffer[512];
	return ReadProcessMemory(handle, (LPVOID)ModuleHandle, g_arrModuleBuffer, sizeof(g_arrModuleBuffer), 0);
}
void M::Install()
{

	
	HMODULE hMods[1024];
	DWORD cbNeeded;
	if (EnumProcessModules(handle, hMods,sizeof(hMods), &cbNeeded))
	{
		MODULEINFO miModInfo;
		DWORD cb_sizeof;
		GetModuleInformation(handle, hMods[0], &miModInfo, sizeof(MODULEINFO));
		if (SelectModule((LPCVOID)hMods[0]))
		{
			BaseAddress = (QWORD)miModInfo.lpBaseOfDll;
			SizeOfImage = miModInfo.SizeOfImage;
			fprintf(stderr, "\n[AA3]SelectModule Success \n");
		}
		else {
			fprintf(stderr, "\n[AA3]SelectModule: \n");
		//	exit(1);
		}
	}
	else {
		fprintf(stderr, "\n[AA3]EnumProcessModules Fail \n");
	}
	
}

bool M::IsGame()
{
	/*
	if (!BaseAddress)
		return false;
	UWorld = Meme.Read<DWORD64>(BaseAddress + offset_UWorld);
	if (!UWorld)
		return false;
	GameInstance = Meme.Read<DWORD64>(UWorld + 0x140);
	if (!GameInstance)
		return false;
	ULocalPlayer = Meme.Read<DWORD64>(GameInstance + 0x38);
	if (!ULocalPlayer)
		return false;
	LocalPlayer = Meme.Read<DWORD64>(ULocalPlayer);
	if (!LocalPlayer)
		return false;
	ViewPortClient = Meme.Read<DWORD64>(LocalPlayer + 0x58);
	if (!ViewPortClient)
		return false;
	Pword = Meme.Read<DWORD64>(ViewPortClient + 0x80);
	if (!Pword)
		return false;
		*/
	return true;
}

void M::Update()
{
	D::isFinding = true;
	//while (1) {
		try
		{
			UWorld = Mem::Read<QWORD>(BaseAddress + 0x3CA74A8);
			GNames = Mem::Read<QWORD>(BaseAddress + 0x3BAA290);
			auto uUWorld = (uintptr_t)UWorld;
			GameInstance = Mem::Read<QWORD>(uUWorld + 0x0140);
			auto uGameInstance = (uintptr_t)GameInstance;
			ULocalPlayer = Mem::Read<QWORD>(uGameInstance + 0x0038);
			auto uULocalPlayer = (uintptr_t)ULocalPlayer;
			LocalPlayer = Mem::Read<QWORD>(uULocalPlayer);
			auto uLocalPlayer = (uintptr_t)LocalPlayer;
			ViewPortClient = Mem::Read<QWORD>(uLocalPlayer + 0x0058);
			auto uViewPortClient = (uintptr_t)ViewPortClient;
			Pword = Mem::Read<QWORD>(uViewPortClient + 0x0080);
			auto uPword = (uintptr_t)Pword;
			ULevel = Mem::Read<QWORD>(uPword + 0x30);
			auto uULevel = (uintptr_t)ULevel;
			EntityList = Mem::Read<QWORD>(uULevel + 0xA0);
			auto uEntityList = (uintptr_t)EntityList;
			LocalPos = Mem::Read<D3DXVECTOR3>(LocalPlayer + 0x70);
			EntityCount = Mem::Read<int>(ULevel + 0xA8);
			LocalPlayerController = Mem::Read<QWORD>(uLocalPlayer + 0x30);
			LocalrootComponent = Mem::Read<QWORD>(uLocalPlayer + 0x180);
			CameraManager = Mem::Read<QWORD>(LocalPlayerController + 0x0438);
			if (LocalPlayerController)
				CameraCache = Mem::Read<FCameraCacheEntry>(CameraManager + 0x950);
			Myangles = Mem::Read<D3DXVECTOR3>(LocalPlayerController + 0x3D0);
			printf("UWorld : %I64d \n", UWorld);
			printf("LocalPlayer : %I64d \n", LocalPlayer);
			printf("LocalPlayerController : %I64d \n", LocalPlayerController);
			//	fprintf(stderr, "\n UWorld %d \n", UWorld);


				//fprintf(stderr, "temp %f %f %f \n", temp.x, temp.y, temp.z);
				/*
				UWorld = Meme.Read<DWORD64>(BaseAddress + 0x37D0528);
				fprintf(stderr, "\n UWorld %d \n", UWorld);
				GameInstance = Meme.Read<DWORD64>(UWorld + 0x0140);
				fprintf(stderr, "\n GameInstance %d \n", GameInstance);
				ULocalPlayer = Meme.Read<DWORD64>(GameInstance + 0x0038);
				fprintf(stderr, "\n ULocalPlayer %d \n", ULocalPlayer);
				LocalPlayer = Meme.Read<DWORD64>(ULocalPlayer + 0x0000);
				fprintf(stderr, "\n LocalPlayer %d \n", LocalPlayer);
				ViewPortClient = Meme.Read<DWORD64>(LocalPlayer + 0x0058);
				fprintf(stderr, "\n ViewPortClient %d \n", ViewPortClient);
				Pword = Meme.Read<DWORD64>(ViewPortClient + 0x0080);
				fprintf(stderr, "\n Pword %d \n", Pword);
				ULevel = Meme.Read<DWORD64>(Pword + 0x0030);
				fprintf(stderr, "\n ULevel %d \n", ULevel);
				EntityList = Meme.Read<DWORD64>(ULevel + 0xA0);
				fprintf(stderr, "\n EntityList %d \n", EntityList);

				DWORD oldProtect;
				VirtualProtectEx(handle, (LPVOID)0x37D0528, sizeof(uintptr_t), PAGE_EXECUTE_READWRITE, &oldProtect);//设置具有读写权限
				if(ReadProcessMemory(handle, (LPCVOID)(0x37D0528), &UWorld, sizeof(uintptr_t), NULL) &&
				ReadProcessMemory(handle, (LPCVOID)(UWorld + 0x0140), &GameInstance, sizeof(uintptr_t), NULL) &&
				ReadProcessMemory(handle, (LPCVOID)(GameInstance + 0x038), &ULocalPlayer, sizeof(uintptr_t), NULL) &&
				ReadProcessMemory(handle, (LPCVOID)(ULocalPlayer), &LocalPlayer, sizeof(uintptr_t), NULL) &&
				ReadProcessMemory(handle, (LPCVOID)(LocalPlayer + 0x30), &PlayerController, sizeof(uintptr_t), NULL) &&
				ReadProcessMemory(handle, (LPCVOID)(LocalPlayer + 0x80), &Pword, sizeof(uintptr_t), NULL) &&
				ReadProcessMemory(handle, (LPCVOID)(Pword + 0x30), &ULevel, sizeof(uintptr_t), NULL) &&
				ReadProcessMemory(handle, (LPCVOID)(ULevel + 0xA0), &EntityList, sizeof(uintptr_t), NULL))
					fprintf(stderr, "\n[AA3]Success! %d \n", handle);
				VirtualProtectEx(handle, (LPVOID)0x37D0528, sizeof(uintptr_t), oldProtect, NULL); //还原权限



			//	ReadProcessMemory(handle, (LPCVOID)(0x37D0528), &UWorld, sizeof(uintptr_t), NULL);
				UWorld = Meme.Read<DWORD64>(BaseAddress + 0x37D0528);

				if (UWorld)
				{
					GameInstance = Meme.Read<DWORD64>(UWorld + 0x140);
					if (!GameInstance)
						fprintf(stderr, "\n[AA3]GameInstance %d \n", GameInstance);
					if (GameInstance)
					{
						ULocalPlayer = Meme.Read<DWORD64>(GameInstance + 0x38);
						if (ULocalPlayer)
						{
							LocalPlayer = Meme.Read<DWORD64>(ULocalPlayer);
				//			LocalPlayer = *(DWORD64*)(ULocalPlayer);
							if (!LocalPlayer)
								throw std::runtime_error("[AA3]LocalPlayer failed (╬▔＾▔)  \n");
							ViewPortClient = Meme.Read<DWORD64>(ULocalPlayer + 0x58);
							if (!ViewPortClient)
								throw std::runtime_error("[AA3]ViewPortClient failed (╬▔＾▔)  \n");
							Pword = Meme.Read<DWORD64>(LocalPlayer + 0x80);
							if (!Pword)
								throw std::runtime_error("[AA3]Pword failed (╬▔＾▔)  \n");
							ULevel = Meme.Read<DWORD64>(Pword + 0x30);
							if (!ULevel)
								throw std::runtime_error("[AA3]ULevel failed (╬▔＾▔)  \n");
							PlayerController = Meme.Read<DWORD64>(LocalPlayer + 0x3A0);
							if (!PlayerController)
								throw std::runtime_error("[AA3]PlayerController failed (╬▔＾▔)  \n");
							EntityList = Meme.Read<DWORD64>(ULevel + 0xA0);
							if (!EntityList)
								throw std::runtime_error("[AA3]EntityList failed (╬▔＾▔)  \n");
					//		fprintf(stderr, "\n[AA3]Success! %d \n", EntityList);

						}

					}
				}
				*/

				//	M::EntityCount = Meme.Read<DWORD64>(M::ULevel + 0xA8);

		}
		catch (const std::exception& ex) {
			fprintf(stderr, ex.what());
			fprintf(stderr, "\n[AA3]GetLastError: %X\n", GetLastError());
			//	exit(1);
		}
		Sleep(1);
	//}
	D::isFinding = false;
}