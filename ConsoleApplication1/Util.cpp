#include "stdafx.h"
#include <ntstatus.h>
#include <Windows.h>
#include <iostream>
#include <fstream>
#include <string>/*
extern QWORD BaseAddress;
std::string D::GetNameFromId(int ID)
{
	DWORD_PTR fNamePtr = Mem::Read<DWORD_PTR>(Mem::Read<DWORD_PTR>(BaseAddress + 0x36D8590) + int(ID / 0x4000) * 8);
	DWORD_PTR fName = Mem::Read<DWORD_PTR>(fNamePtr + 8 * int(ID % 0x4000));
	char name[64] = { NULL };
	if (ReadProcessMemory(handle,(LPVOID)(fName + 16), name, sizeof(name) - 2, NULL) != 0)
		return std::string(name);
	return std::string("NULL");
}

void D::CacheNames()
{
	string name = "";
	for (int i = 0, c1 = 0, c2 = 0, c3 = 0, c4 = 0, c5 = 0, c6 = 0; i < 200000; i++)
	{
		if (c1 == 6 && c2 == 3 && c3 == 4 && c4 == 5 && c5 == 3 && boat != 0 && c6 == 2)
		{
			cout << "IDs retrieved" << endl;
			return;
		}
		name = GetNameFromId(i);
		if (name == "PlayerFemale_A" || name == "PlayerFemale_A_C" || name == "PlayerMale_A" || name == "PlayerMale_A_C" || name == "Male_B_Coat_D" || name == "Male_B_Coat_N")
		{
			cout << name << " " << i << endl;
			ActorIds[c1++] = i;
		}
		if (name == "Uaz_A_01_C" || name == "Uaz_B_01_C" || name == "Uaz_C_01_C")
		{
			uaz[c2++] = i;
			cout << name << " " << i << endl;
		}
		if (name == "Dacia_A_01_C" || name == "Dacia_A_02_C" || name == "Dacia_A_03_C" || name == "Dacia_A_04_C")
		{
			dacia[c3++] = i;
			cout << name << " " << i << endl;
		}
		if (name == "ABP_Motorbike_03_C" || name == "ABP_Motorbike_04_C" || name == "BP_Motorbike_03_C" || name == "BP_Motorbike_04_C" || name == "ABP_Motorbike_03_Sidecart_C")
		{
			cout << name << " " << i << endl;
			motorbike[c4++] = i;
		}
		if (name == "Buggy_A_01_C" || name == "Buggy_A_02_C" || name == "Buggy_A_03_C")
		{
			cout << name << " " << i << endl;
			buggy[c5++] = i;
		}
		if (name == "Boat_PG117_C")
		{
			cout << name << " " << i << endl;
			boat = i;
		}
		if (name == "DroppedItemInteractionComponent" || name == "DroppedItemGroup")
		{
			cout << name << " " << i << endl;
			itemtype[c6++] = i;
		}
	}
}*/