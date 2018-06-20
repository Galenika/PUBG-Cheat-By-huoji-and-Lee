#pragma once

struct Variables
{
	Variables()
	{

	}
	struct
	{

		bool	Opened = false;
	} Menu;
	struct
	{
		bool	VisualEnable = true;
		bool	NearPlayerDetectWarning = true; 
		bool	DrawCrosshair;
		bool	Box = true;
		bool	Ske = true;
		bool	Health = true;
		bool	isShowTeammate;
		bool	isShowCar = true;
		bool	isShowGun = true;
		int		StrShow = true;
		int		wespdt = 300;
		float	RedWarnDST = 150;
		float	YellowWarnDST = 300;
		int		maxDST = 1500;
		bool isFilter = false;
		bool	isShowItem = true;
		bool	isShowAmmo = false;
		bool	isShowPart = false;
		bool	isShowDrug = true;
		bool	isShowEquip = false;
		bool	isShowOther = false;
		bool isShowPing = false;
	} Visual;
	struct
	{
		bool	AimbotEnable = false;
		bool	AimMother;
		bool		RandBone;
		int		AimKey;
		int		Bone;
		float	FOV;
		float   Dist;
		
	} Aimbot;
	struct
	{
		bool	MiscEnable = false;
		bool	FastUseAndInWaterUser;
		bool	NoSway;
		bool	NoGravity;
		bool	HitDamage;
		bool	SpeedHack;
		bool	RageSpeedHack;
		bool	FastProne;
		float	MaxSpeed;
		int   FuckUp;
	} Misc;
	struct
	{
		LPSTR GCmdLine;
		std::string key;
		bool Flags;
		std::string Time;
	} Lazy;
};
enum EMovementMode
{
	MOVE_None = 0,
	MOVE_Walking = 1,
	MOVE_NavWalking = 2,
	MOVE_Falling = 3,
	MOVE_Swimming = 4,
	MOVE_Flying = 5,
	MOVE_Custom = 6,
	MOVE_MAX = 7
};

extern Variables Vars;
static const char* AimBones[] =
{
	"Head",
	"Neck",
	"Chest"
};
static const char* keyNames[] =
{
	"",
	"Mouse 1",
	"Mouse 2",
	"Cancel",
	"Middle Mouse",
	"Mouse 4",
	"Mouse 5",
	"",
	"Backspace",
	"Tab",
	"",
	"",
	"Clear",
	"Enter",
	"",
	"",
	"Shift",
	"Control",
	"Alt",
	"Pause",
	"Caps",
	"",
	"",
	"",
	"",
	"",
	"",
	"Escape",
	"",
	"",
	"",
	"",
	"Space",
	"Page Up",
	"Page Down",
	"End",
	"Home",
	"Left",
	"Up",
	"Right",
	"Down",
	"",
	"",
	"",
	"Print",
	"Insert",
	"Delete",
	"",
	"0",
	"1",
	"2",
	"3",
	"4",
	"5",
	"6",
	"7",
	"8",
	"9",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"A",
	"B",
	"C",
	"D",
	"E",
	"F",
	"G",
	"H",
	"I",
	"J",
	"K",
	"L",
	"M",
	"N",
	"O",
	"P",
	"Q",
	"R",
	"S",
	"T",
	"U",
	"V",
	"W",
	"X",
	"Y",
	"Z",
	"",
	"",
	"",
	"",
	"",
	"Numpad 0",
	"Numpad 1",
	"Numpad 2",
	"Numpad 3",
	"Numpad 4",
	"Numpad 5",
	"Numpad 6",
	"Numpad 7",
	"Numpad 8",
	"Numpad 9",
	"Multiply",
	"Add",
	"",
	"Subtract",
	"Decimal",
	"Divide",
	"F1",
	"F2",
	"F3",
	"F4",
	"F5",
	"F6",
	"F7",
	"F8",
	"F9",
	"F10",
	"F11",
	"F12",

};