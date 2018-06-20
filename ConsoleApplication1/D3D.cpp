#include "stdafx.h"
#include <TlHelp32.h>
#include <math.h>
#include <time.h>
#include <tchar.h>
#include <thread>
#include "XorStr.h"
#include "strenc.h"
#include "chars.h"
#define PI 3.1415926535898
const char* rC;
int ActorIds[6];
int uaz[3];
int dacia[4];
int motorbike[5];
int buggy[3];
int boat = 0;
int itemtype[3];
extern QWORD SizeOfImage;
extern QWORD ULocalPlayer;
extern QWORD ViewPortClient;
extern QWORD LocalPlayer;
extern QWORD ULevel;
extern QWORD PlayerController;
extern QWORD EntityList;
extern QWORD UWorld;
extern QWORD GameInstance;
extern QWORD Pword;
extern QWORD ModuleHandle;
extern int EntityCount;
extern QWORD BaseAddress;
extern QWORD LocalrootComponent;
extern QWORD LocalPlayerController;
extern D3DXVECTOR3 Myangles;
extern QWORD CameraManager;
extern D3DXVECTOR3 LocalPos;
extern FCameraCacheEntry CameraCache;
#define b2c(x) (const char*)x
bool canExit = false, isRendering = false, isAiming = false;
struct gUchar
{
	WCHAR Text[128];
};
#pragma region 3DUtil
std::string GetNameFromId(int ID)
{
	static auto uBaseAddress = (uintptr_t)BaseAddress;
	DWORD_PTR fNamePtr = Mem::Read<DWORD_PTR>(Mem::Read<DWORD_PTR>(BaseAddress + 0x3BAA290) + int(ID / 0x4000) * 8);
	DWORD_PTR fName = Mem::Read<DWORD_PTR>(fNamePtr + 8 * int(ID % 0x4000));
	char name[64] = { NULL };
	if (fName && ReadProcessMemory(handle, (LPVOID)(fName + 16), name, sizeof(name) - 2, NULL) != 0)
		return std::string(name);
	return std::string(strenc("NULL"));
}
D3DXVECTOR3 TransformCoordinate(D3DXVECTOR3 &vCoordinate, D3DXMATRIX &mTransform)
{
	D3DXVECTOR4 vWorking;

	vWorking.x = (vCoordinate.x * mTransform._11) + (vCoordinate.y * mTransform._21) + (vCoordinate.z * mTransform._31) + mTransform._41;
	vWorking.y = (vCoordinate.x * mTransform._12) + (vCoordinate.y * mTransform._22) + (vCoordinate.z * mTransform._32) + mTransform._42;
	vWorking.z = (vCoordinate.x * mTransform._13) + (vCoordinate.y * mTransform._23) + (vCoordinate.z * mTransform._33) + mTransform._43;
	vWorking.w = 1 / ((vCoordinate.x * mTransform._14) + (vCoordinate.y * mTransform._24) + (vCoordinate.z * mTransform._34) + mTransform._44);

	return D3DXVECTOR3(vWorking.x * vWorking.w, vWorking.y * vWorking.w, vWorking.z * vWorking.w);
}
void D::DrawLines(D3DXVECTOR2 point0[5])
{
	if (!point0[2] || !point0[3] || !point0[4] || !point0[5])
		return;

	for (int x = 0; x < 5 - 1; x++)
		D::DrawLine(point0[x].x, point0[x + 1].y, point0[x].x, point0[x + 1].y, D3DCOLOR_RGBA(255, 0, 0, 255));
}
void D::Draw3DBox(D3DXVECTOR3 rotat, D3DXVECTOR3 location, float fov, bool isCar, D3DXVECTOR3 loc, D3DXVECTOR3 rot)
{
	float l, w, h, o;
	l = 60.f;
	w = 60.f;
	h = 160.f;
	o = 50.f; //box size for standing player

	float zOffset = -100.f;

	if (isCar) //vehicles have a bigger box
	{
		zOffset = 50;
		l = 200.f; w = 160.f; h = 80.f; o = 0.f;
	}

	//build box
	D3DXVECTOR3 p00 = D3DXVECTOR3(o, -w / 2, 0.f);
	D3DXVECTOR3 p01 = D3DXVECTOR3(o, w / 2, 0.f);
	D3DXVECTOR3 p02 = D3DXVECTOR3(o - l, w / 2, 0.f);
	D3DXVECTOR3 p03 = D3DXVECTOR3(o - l, -w / 2, 0.f);

	//rotate rectangle to match actor rotation
	float theta1 = 2.0f * (float)PI * (rot.y) / 180.0f;
	D3DXMATRIX rotM; // rotate around Z-axis
	D3DXMatrixRotationZ(&rotM, (float)(theta1 / 2));
	D3DXVECTOR3 curPos(loc.x, loc.y, loc.z + zOffset);
	p00 = TransformCoordinate(p00, rotM) + curPos;
	p01 = TransformCoordinate(p01, rotM) + curPos;
	p02 = TransformCoordinate(p02, rotM) + curPos;
	p03 = TransformCoordinate(p03, rotM) + curPos;

	D3DXVECTOR2 s00, s01, s02, s03;
	toScreen(rotat, location, fov, p00, s00);
	toScreen(rotat, location, fov, p01, s01);
	toScreen(rotat, location, fov, p02, s02);
	toScreen(rotat, location, fov, p03, s03);
	D3DXVECTOR2 square0[5] = { s00, s01, s02, s03, s00 };

	DrawLines(square0);

	// top rectangle
	p00.z += h;
	p01.z += h;
	p02.z += h;
	p03.z += h;
	toScreen(rotat, location, fov, p00, s00);
	toScreen(rotat, location, fov, p01, s01);
	toScreen(rotat, location, fov, p02, s02);
	toScreen(rotat, location, fov, p03, s03);

	D3DXVECTOR2 square1[5] = { s00, s01, s02, s03, s00 };
	DrawLines(square0);

	// upper/lower rectangles get connected
	DrawLines(square1);
}
void CacheNames()
{
	string name = "";
	for (int i = 0, c1 = 0, c2 = 0, c3 = 0, c4 = 0, c5 = 0, c6 = 0; i < 200000; i++)
	{
		if (c1 == 6 && c2 == 3 && c3 == 4 && c4 == 5 && c5 == 3 && boat != 0 && c6 == 2)
		{
			return;
		}
		name = GetNameFromId(i);
		if (name == strenc("PlayerFemale_A") || name == strenc("PlayerFemale_A_C") || name == strenc("PlayerMale_A") || name == strenc("PlayerMale_A_C") || name == strenc("Male_B_Coat_D") || name == strenc("Male_B_Coat_N"))
		{
			ActorIds[c1++] = i;
		}
		if (name == strenc("Uaz_A_01_C") || name == strenc("Uaz_B_01_C") || name == strenc("Uaz_C_01_C"))
		{
			uaz[c2++] = i;
		}
		if (name == strenc("Dacia_A_01_C") || name == strenc("Dacia_A_02_C") || name == strenc("Dacia_A_03_C") || name == strenc("Dacia_A_04_C"))
		{
			dacia[c3++] = i;
		}
		if (name == strenc("ABP_Motorbike_03_C") || name == strenc("ABP_Motorbike_04_C") || name == strenc("BP_Motorbike_03_C") || name == strenc("BP_Motorbike_04_C") || name == strenc("ABP_Motorbike_03_Sidecart_C"))
		{
			motorbike[c4++] = i;
		}
		if (name == strenc("Buggy_A_01_C") || name == strenc("Buggy_A_02_C") || name == strenc("Buggy_A_03_C"))
		{
			buggy[c5++] = i;
		}
		if (name == strenc("Boat_PG117_C"))
		{
			boat = i;
		}
		if (name == strenc("DroppedItemInteractionComponent") || name == strenc("DroppedItemGroup") || name == strenc("AddItem"))
		{
			itemtype[c6++] = i;
		}
	}
}
#pragma endregion
#pragma region 字符串转换
std::wstring utf8_to_utf16(const std::string& utf8)
{
	std::vector<unsigned long> unicode;
	size_t i = 0;
	while (i < utf8.size())
	{
		unsigned long uni;
		size_t todo;
		bool error = false;
		unsigned char ch = utf8[i++];
		if (ch <= 0x7F)
		{
			uni = ch;
			todo = 0;
		}
		else if (ch <= 0xBF)
		{
			throw std::logic_error(charenc("ERROR"));
		}
		else if (ch <= 0xDF)
		{
			uni = ch & 0x1F;
			todo = 1;
		}
		else if (ch <= 0xEF)
		{
			uni = ch & 0x0F;
			todo = 2;
		}
		else if (ch <= 0xF7)
		{
			uni = ch & 0x07;
			todo = 3;
		}
		else
		{
			throw std::logic_error(charenc("ERROR"));
		}
		for (size_t j = 0; j < todo; ++j)
		{
			if (i == utf8.size())
				throw std::logic_error(charenc("ERROR"));
			unsigned char ch = utf8[i++];
			if (ch < 0x80 || ch > 0xBF)
				throw std::logic_error(charenc("ERROR"));
			uni <<= 6;
			uni += ch & 0x3F;
		}
		if (uni >= 0xD800 && uni <= 0xDFFF)
			throw std::logic_error(charenc("ERROR"));
		if (uni > 0x10FFFF)
			throw std::logic_error(charenc("ERROR"));
		unicode.push_back(uni);
	}
	std::wstring utf16;
	for (size_t i = 0; i < unicode.size(); ++i)
	{
		unsigned long uni = unicode[i];
		if (uni <= 0xFFFF)
		{
			utf16 += (wchar_t)uni;
		}
		else
		{
			uni -= 0x10000;
			utf16 += (wchar_t)((uni >> 10) + 0xD800);
			utf16 += (wchar_t)((uni & 0x3FF) + 0xDC00);
		}
	}
	return utf16;
}
const wchar_t *GetWC(const char *c)
{
	const size_t cSize = strlen(c) + 1;
	wchar_t* wc = new wchar_t[cSize];
	mbstowcs(wc, c, cSize);

	return wc;
}
char* WCharToChar(WCHAR *s) {
	int w_nlen = WideCharToMultiByte(CP_ACP, 0, s, -1, NULL, 0, NULL, false);
	char *ret = new char[w_nlen];
	memset(ret, 0, w_nlen);
	WideCharToMultiByte(CP_ACP, 0, s, -1, ret, w_nlen, NULL, false);
	return ret;
}
char * randomChinese(size_t len) {
	srand(time(NULL));
	int mlen = sizeof(uint8_t) * len * 2;
	uint8_t *b = (uint8_t *)malloc(mlen + 1);
	for (int i = 0; i < mlen; i++)
	{
		if (i % 2 == 0) {
			b[i] = rand() % (0xD7 - 0xB0 + 1) + 0xB0;
		}
		else {
			b[i] = rand() % (0xFE - 0xA1 + 1) + 0xA1;
		}
	}
	b[mlen] = '\0';
	char * title = (char *)b;
	return title;
}
static size_t jksn_utf16_to_utf8(const uint16_t *utf16str, char *utf8str, size_t utf16size) {
	size_t reslen = 0;
	while (utf16size) {
		if (utf16str[0] < 0x80) {
			if (utf8str)
				utf8str[reslen] = utf16str[0];
			reslen++;
			utf16str++;
			utf16size--;
		}
		else if (utf16str[0] < 0x800) {
			if (utf8str) {
				utf8str[reslen] = utf16str[0] >> 6 | 0xc0;
				utf8str[reslen + 1] = (utf16str[0] & 0x3f) | 0x80;
			}
			reslen += 2;
			utf16str++;
			utf16size--;
		}
		else if ((utf16str[0] & 0xf800) != 0xd800) {
			if (utf8str) {
				utf8str[reslen] = utf16str[0] >> 12 | 0xe0;
				utf8str[reslen + 1] = ((utf16str[0] >> 6) & 0x3f) | 0x80;
				utf8str[reslen + 2] = (utf16str[0] & 0x3f) | 0x80;
			}
			reslen += 3;
			utf16str++;
			utf16size--;
		}
		else if (utf16size != 1 && (utf16str[0] & 0xfc00) == 0xd800 && (utf16str[1] & 0xfc00) == 0xdc00) {
			uint32_t ucs4 = ((uint32_t)(utf16str[0] & 0x3ff) << 10 | (uint32_t)(utf16str[1] & 0x3ff)) + 0x10000;
			if (utf8str) {
				utf8str[reslen] = ucs4 >> 18 | 0xf0;
				utf8str[reslen + 1] = ((ucs4 >> 12) & 0x3f) | 0x80;
				utf8str[reslen + 2] = ((ucs4 >> 6) & 0x3f) | 0x80;
				utf8str[reslen + 3] = (ucs4 & 0x3f) | 0x80;
			}
			reslen += 4;
			utf16str += 2;
			utf16size -= 2;
		}
		else {
			if (utf8str) {
				utf8str[reslen] = 0xef;
				utf8str[reslen + 1] = 0xbf;
				utf8str[reslen + 2] = 0xbd;
			}
			reslen += 3;
			utf16str++;
			utf16size--;
		}
	}
	utf8str[reslen] = '\0';
	return reslen;
}
#pragma endregion
#pragma region Util
WNDPROC OldWndProc = nullptr;
/*
指定窗口的扩展风格。该参数可以是下列值：
WS_EX_NODRAG:防止窗口被移动
WS_EX_ACCEPTFILES：指定以该风格创建的窗口接受一个拖拽文件。
WS_EX_APPWINDOW：当窗口可见时，将一个顶层窗口放置到任务条上。
WS_EX_CLIENTEDGE：指定窗口有一个带阴影的边界。
WS_EX_CONTEXTHELP：在窗口的标题条包含一个问号标志。当用户点击了问号时，鼠标光标变为一个问号的指针、如果点击了一个子窗口，则子窗口接收到WM_HELP消息。子窗口应该将这个消息传递给父窗口过程，父窗口再通过HELP_WM_HELP命令调用WinHelp函数。这个Help应用程序显示一个包含子窗口帮助信息的弹出式窗口。 WS_EX_CONTEXTHELP不能与WS_MAXIMIZEBOX和WS_MINIMIZEBOX同时使用。
WS_EX_CONTROLPARENT：允许用户使用Tab键在窗口的子窗口间搜索。
WS_EX_DLGMODALFRAME：创建一个带双边的窗口；该窗口可以在dwStyle中指定WS_CAPTION风格来创建一个标题栏。
WS_EX_LAYERED：创建一个分层窗口
WS_EX_LEFT：窗口具有左对齐属性，这是缺省设置的。
WS_EX_LEFTSCROLLBAR：如果外壳语言是如Hebrew，Arabic，或其他支持reading order alignment的语言，则标题条（如果存在）则在客户区的左部分。若是其他语言，在该风格被忽略并且不作为错误处理。
WS_EX_LTRREADING：窗口文本以LEFT到RIGHT（自左向右）属性的顺序显示。这是缺省设置的。
WS_EX_MDICHILD：创建一个MDI子窗口。
WS_EX_NOPATARENTNOTIFY：指明以这个风格创建的窗口在被创建和销毁时不向父窗口发送WM_PARENTNOTFY消息。
WS_EX_OVERLAPPEDWINDOW：WS_EX_CLIENTEDGE和WS_EX_WINDOWEDGE的组合。
WS_EX_PALETTEWINDOW：WS_EX_WINDOWEDGE, WS_EX_TOOLWINDOW和WS_WX_TOPMOST风格的组合WS_EX_RIGHT:窗口具有普通的右对齐属性，这依赖于窗口类。只有在外壳语言是如Hebrew,Arabic或其他支持读顺序对齐（reading order alignment）的语言时该风格才有效，否则，忽略该标志并且不作为错误处理。
WS_EX_RIGHTSCROLLBAR：垂直滚动条在窗口的右边界。这是缺省设置的。
WS_EX_RTLREADING：如果外壳语言是如Hebrew，Arabic，或其他支持读顺序对齐（reading order alignment）的语言，则窗口文本是一自左向右）RIGHT到LEFT顺序的读出顺序。若是其他语言，在该风格被忽略并且不作为错误处理。
WS_EX_STATICEDGE：为不接受用户输入的项创建一个3一维边界风格
WS_EX_TOOLWINDOW：创建工具窗口，即窗口是一个游动的工具条。工具窗口的标题条比一般窗口的标题条短，并且窗口标题以小字体显示。工具窗口不在任务栏里显示，当用户按下alt+Tab键时工具窗口不在对话框里显示。如果工具窗口有一个系统菜单，它的图标也不会显示在标题栏里，但是，可以通过点击鼠标右键或Alt+Space来显示菜单。
*/
DWORD rz(int z) {
	switch (z) {
	case 1:
		return WS_EX_STATICEDGE;
	case 2:
		return WS_EX_ACCEPTFILES;
	case 3:
		return WS_EX_CONTEXTHELP;
	case 4:
		return WS_EX_RIGHT;
	case 5:
		return WS_EX_LEFT;
	case 6:
		return WS_EX_LEFTSCROLLBAR;
	case 7:
		return WS_EX_CONTROLPARENT;
	case 8:
		return WS_EX_PALETTEWINDOW;
	default:
		return WS_EX_WINDOWEDGE;
	}
}
DWORD sbbe() {
	srand(unsigned(time(0)));
	int z = 1 + rand() % 8;
	srand(unsigned(time(0)));
	int x = 1 + rand() % 8;
	srand(unsigned(time(0)));
	int c = 1 + rand() % 8;
	return rz(z) | rz(x) | rz(c);
}
#pragma endregion
#pragma region WndProc
LRESULT __stdcall D::HookedWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{

	ImGuiIO& io = ImGui::GetIO();

	switch (uMsg)
	{
	case WM_SIZE:
		if (p_Device != NULL && wParam != SIZE_MINIMIZED)
		{
			ImGui_ImplDX9_InvalidateDeviceObjects();
			p_Params.BackBufferWidth = LOWORD(lParam);
			p_Params.BackBufferHeight = HIWORD(lParam);
			HRESULT hr = p_Device->Reset(&p_Params);
			if (hr == D3DERR_INVALIDCALL)
				IM_ASSERT(0);
			ImGui_ImplDX9_CreateDeviceObjects();
		}
		return 0;
		//	case WM_PAINT:
		//		D::Render();
		//		break;
	case WM_CREATE:
		//DwmExtendFrameIntoClientArea(hWnd, &D::Margin);
		break;
	case WM_DESTROY:
		D::p_Line->Release();
		PostQuitMessage(1);
		return 0;
	case WM_LBUTTONDOWN:
		io.MouseDown[0] = true;
		break;
	case WM_LBUTTONUP:
		io.MouseDown[0] = false;
		break;
	case WM_RBUTTONDOWN:
		io.MouseDown[1] = true;
		break;
	case WM_RBUTTONUP:
		io.MouseDown[1] = false;
		break;
	case WM_MBUTTONDOWN:
		io.MouseDown[2] = true;
		break;
	case WM_MBUTTONUP:
		io.MouseDown[2] = false;
		break;
	case WM_MOUSEWHEEL:
		io.MouseWheel += GET_WHEEL_DELTA_WPARAM(wParam) > 0 ? 1.0f : -1.0f;
		break;
	case WM_MOUSEMOVE:
		io.MousePos.x = (signed short)(lParam);
		io.MousePos.y = (signed short)(lParam >> 16);
		break;
	case WM_KEYDOWN:
		if (wParam == 52)
			break;
		if (wParam < 256)
			io.KeysDown[wParam] = 1;
		break;
	case WM_KEYUP:
		if (wParam == 52)
			break;
		if (wParam < 256)
			io.KeysDown[wParam] = 0;
		break;
	default:
		//	OutputDebugString(L"2 \n");
		return DefWindowProc(hWnd, uMsg, wParam, lParam);
		break;
	}
	//	OutputDebugString(L"3 \n");
	return 0;

}
#pragma endregion
#pragma region Name2Pid
DWORD GetProcessidFromName(LPCTSTR name)
{
	PROCESSENTRY32 pe;
	DWORD id = 0;
	HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	pe.dwSize = sizeof(PROCESSENTRY32);
	if (!Process32First(hSnapshot, &pe))
		return 0;
	while (1)
	{
		pe.dwSize = sizeof(PROCESSENTRY32);
		if (Process32Next(hSnapshot, &pe) == FALSE)
			break;
		if (_tcscmp(pe.szExeFile, name) == 0)
		{
			id = pe.th32ProcessID;

			break;
		}


	}
	CloseHandle(hSnapshot);
	return id;
}
#pragma endregion
#pragma region SetToTarget
bool isSetWindowToTargeted = true;
void D::SetWindowToTarget()
{
	isSetWindowToTargeted = false;
	while (1) {
		static long flags;
		const wchar_t TslGame[12] = { 0x54,0x73,0x6c,0x47,0x61,0x6d,0x65,0x2e,0x65,0x78,0x65 };
		if (GetProcessidFromName(TslGame) != 0)
		{
			int cxScreen, cyScreen;
			cxScreen = GetSystemMetrics(SM_CXSCREEN);
			cyScreen = GetSystemMetrics(SM_CYSCREEN);
			SetWindowPos(hWnd, HWND_TOPMOST, 0, 0, cxScreen, cyScreen, SWP_SHOWWINDOW);

			static bool is_down = false;
			static bool is_clicked = false;

			if (GetAsyncKeyState(VK_INSERT))
			{
				is_clicked = false;
				is_down = true;
			}
			else if (!GetAsyncKeyState(VK_INSERT) && is_down)
			{
				is_clicked = true;
				is_down = false;
			}
			else
			{
				is_clicked = false;
				is_down = false;
			}

			if (is_clicked)
			{
				Vars.Menu.Opened = !Vars.Menu.Opened;
			}

			flags = GetWindowLong(hWnd, GWL_EXSTYLE);

			if (Vars.Menu.Opened)
			{

				if (flags & WS_EX_LAYERED)
				{

					SetWindowLong(hWnd, GWL_EXSTYLE, GetWindowLong(hWnd, GWL_EXSTYLE) &~WS_EX_LAYERED);
				}

			}
			else {
				if (!(flags & WS_EX_LAYERED))
				{

					SetWindowLong(hWnd, GWL_EXSTYLE, GetWindowLong(hWnd, GWL_EXSTYLE) | WS_EX_LAYERED);
				}

			}
		}
		else {
			ImGui_ImplDX9_Shutdown();
			if (p_Device) p_Device->Release();
			if (p_Object) p_Object->Release();
			if (p_Line) p_Line->Release();
			exit(1);
		}
		UpdateWindow(hWnd);
		Sleep(24);
	}
	isSetWindowToTargeted = true;
}
#pragma endregion
struct SENTRY {
	string searchname;
	wstring outputname;
	SENTRY(const char* search, const WCHAR* output) {
		searchname = search;
		outputname = output;
	}
};
vector<SENTRY> names;
int D::Start()
{
	names.reserve(100);
	names.push_back(SENTRY(charenc("Uaz_A_01_C"), encstr("Uaz1")));
	names.push_back(SENTRY(charenc("Uaz_B_01_C"), encstr("Uaz2")));
	names.push_back(SENTRY(charenc("Uaz_C_01_C"), encstr("Uaz3")));
	names.push_back(SENTRY(charenc("Boat_PG117_C"), encstr("Boat")));
	names.push_back(SENTRY(charenc("Buggy_A_01_C"), encstr("Buggy1")));
	names.push_back(SENTRY(charenc("Buggy_A_02_C"), encstr("Buggy2")));
	names.push_back(SENTRY(charenc("Buggy_A_03_C"), encstr("Buggy3")));
	names.push_back(SENTRY(charenc("Dacia_A_01_C"), encstr("Dacia1")));
	names.push_back(SENTRY(charenc("Dacia_A_02_C"), encstr("Dacia2")));
	names.push_back(SENTRY(charenc("Dacia_A_03_C"), encstr("Dacia3")));
	names.push_back(SENTRY(charenc("Dacia_A_04_C"), encstr("Dacia4")));
	names.push_back(SENTRY(charenc("BP_Motorbike_04_C"), encstr("Moto")));
	names.push_back(SENTRY(charenc("BP_Motorbike_04_SideCar_C"), encstr("Moto")));
	names.push_back(SENTRY(charenc("DeathDropItemPackage_C"), encstr("Death Box")));
	names.push_back(SENTRY(charenc("Carapackage_RedBox_C"), encstr("Care Package")));
	//	names.push_back(SENTRY("DroppedItemGroup", L"物品组"));
		//names.push_back(SENTRY("Item_Back_F_02_Lv2_C", L"背包|Lv2"));
		//names.push_back(SENTRY("Item_Back_C_02_Lv3_C", L"背包|Lv3"));
		//names.push_back(SENTRY("Item_Armor_D_01_Lv2_C", L"防弹背心|Lv2"));
		//names.push_back(SENTRY("Item_Armor_C_01_Lv3_C", L"防弹背心|Lv3"));
		//names.push_back(SENTRY("Item_Head_F_02_Lv2_C", L"头盔|Lv2"));
		//names.push_back(SENTRY("Item_Head_G_01_Lv3_C", L"头盔|Lv3"));
		/*
		names.push_back(SENTRY("Item_Heal_FirstAid_C", L"急救箱"));
		names.push_back(SENTRY("Item_Heal_MedKit_C", L"医疗包"));
		names.push_back(SENTRY("Item_Boost_PainKiller_C", L"止痛药"));
		names.push_back(SENTRY("Item_Boost_EnergyDrink_C", L"能量饮料"));
		names.push_back(SENTRY("Item_Weapon_AK47_C", L"AK47"));
		names.push_back(SENTRY("Item_Weapon_SKS_C", L"SKS"));
		names.push_back(SENTRY("Item_Weapon_SCAR-L_C", L"SCAR"));
		names.push_back(SENTRY("Item_Weapon_Pan_C", L"Pan"));
		names.push_back(SENTRY("Item_Weapon_Kar98k_C", L"98k"));
		names.push_back(SENTRY("Item_Weapon_M16A4_C", L"M16A4"));
		names.push_back(SENTRY("Item_Weapon_HK416_C", L"M416"));
		*/
		//names.push_back(SENTRY("Item_Weapon_Mini14_C", L"Mini14"));
		//names.push_back(SENTRY("Item_Ammo_556mm_C", L"556"));
		//names.push_back(SENTRY("Item_Ammo_762mm_C", L"762"));
		//names.push_back(SENTRY("Item_Attach_Weapon_Muzzle_Suppressor_Large_C", L"AR_Supressor"));
		//names.push_back(SENTRY("Item_Attach_Weapon_Muzzle_FlashHider_SniperRifle_C", L"SR_Supressor"));
		//names.push_back(SENTRY("Item_Attach_Weapon_Muzzle_Compensator_SniperRifle_C", L"SR_Compensator"));
		//names.push_back(SENTRY("Item_Attach_Weapon_Muzzle_Compensator_Large_C", L"AR_Compensator"));
		//names.push_back(SENTRY("Item_Attach_Weapon_Magazine_ExtendedQuickDraw_Large_C", L"AR_Ext_Quickdraw"));
		//names.push_back(SENTRY("Item_Attach_Weapon_Magazine_ExtendedQuickDraw_SniperRifle_C", L"SR_Ext_Quickdraw"));
		//names.push_back(SENTRY("Item_Attach_Weapon_Upper_Aimpoint_C", L"两倍镜"));
		//names.push_back(SENTRY("Item_Attach_Weapon_Upper_ACOG_01_C", L"四倍镜"));
		//names.push_back(SENTRY("Item_Attach_Weapon_Upper_CQBSS_C", L"八倍镜"));
		//names.push_back(SENTRY("Item_Attach_Weapon_Upper_DotSight_01_C", L"红点瞄准镜"));
		//names.push_back(SENTRY("Item_Attach_Weapon_Stock_SniperRifle_CheekPad_C", L"狙击托腮板"));
	rC = randomChinese(10);


	WNDCLASSEX wClass;
	wClass.cbClsExtra = NULL;
	wClass.cbSize = sizeof(WNDCLASSEX);
	wClass.cbWndExtra = NULL;
	wClass.hbrBackground = NULL;
	wClass.hCursor = LoadCursor(0, IDC_ARROW);
	wClass.hIcon = LoadIcon(0, IDI_APPLICATION);
	wClass.hIconSm = LoadIcon(0, IDI_APPLICATION);
	wClass.hInstance = hInstance;
	wClass.lpfnWndProc = D::HookedWndProc;
	wClass.lpszClassName = GetWC(rC); //L"Google Chrome";
	wClass.lpszMenuName = GetWC(rC);//L"Google Chrome";
	wClass.style = CS_VREDRAW | CS_HREDRAW | ACS_TRANSPARENT;

	if (!RegisterClassEx(&wClass))
		exit(1);

	cx = GetSystemMetrics(SM_CXSCREEN) * 2;
	cy = GetSystemMetrics(SM_CYSCREEN) * 2;
	Width = cx;
	Height = cy;
	hWnd = CreateWindowEx(/*WS_EX_TOPMOST |*/ WS_EX_TRANSPARENT | WS_EX_LAYERED | WS_EX_TOOLWINDOW | sbbe(), GetWC(rC), GetWC(rC), WS_POPUP, 1, 1, Width, Height, 0, 0, 0, 0);
	//SetLayeredWindowAttributes(hWnd, 0, 1.0f, LWA_ALPHA);
	//SetLayeredWindowAttributes(hWnd, 0, RGB(0, 0, 0), LWA_COLORKEY);
	SetLayeredWindowAttributes(hWnd, RGB(0, 0, 0), 0, LWA_ALPHA | LWA_COLORKEY);
	SetWindowLong(hWnd, GWL_EXSTYLE, GetWindowLong(hWnd, GWL_EXSTYLE) &~WS_EX_APPWINDOW);
	MARGINS margin = { -1 };
	DwmExtendFrameIntoClientArea(hWnd, &margin);
	ShowWindow(hWnd, SW_SHOW);
	DirectXInit(hWnd);
	M::Install();
	CacheNames();
	//if (isSetWindowToTargeted) {
	std::thread UhWndz(D::SetWindowToTarget);
	UhWndz.detach();
	//}


	//update mem
	while (Message.message != WM_QUIT)
	{
		if (PeekMessage(&Message, hWnd, 0, 0, PM_REMOVE))
		{
			DispatchMessage(&Message);
			TranslateMessage(&Message);
			continue;
		}
		else
		{
			if (!isFinding) {
				std::thread UMeM(M::Update);
				UMeM.detach();
			}

			//start aimbot
			if (!isAiming) {
				std::thread UAimbot(D::AimBot);
				UAimbot.detach();
			}

			//start render
			if (!isRendering) {
				std::thread URenderz(D::zRender);
				URenderz.join();
			}
		}
	}
	ImGui_ImplDX9_Shutdown();
	if (p_Device) p_Device->Release();
	if (p_Object) p_Object->Release();
	if (p_Line) p_Line->Release();
	return 0;
}

int D::DirectXInit(HWND hWnd)
{
	SetPriorityClass(GetCurrentProcess(), HIGH_PRIORITY_CLASS);
	//if (FAILED(Direct3DCreate9(D3D_SDK_VERSION, &p_Object)))
		//exit(1);
	LPDIRECT3D9 pD3D;
	if ((pD3D = Direct3DCreate9(D3D_SDK_VERSION)) == NULL)
	{
		UnregisterClass(wcharenc("XGui"), hInstance);
		exit(1);
	}
	ZeroMemory(&p_Params, sizeof(p_Params));
	p_Params.Windowed = TRUE;
	p_Params.SwapEffect = D3DSWAPEFFECT_FLIP;
	p_Params.hDeviceWindow = hWnd;
	p_Params.MultiSampleQuality = D3DMULTISAMPLE_NONE;
	p_Params.BackBufferCount = 0;
	p_Params.BackBufferFormat = D3DFMT_A8R8G8B8;
	p_Params.BackBufferWidth = Width;
	p_Params.BackBufferHeight = Height;
	p_Params.EnableAutoDepthStencil = TRUE;
	p_Params.AutoDepthStencilFormat = D3DFMT_D16;
	p_Params.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;
	p_Params.FullScreen_RefreshRateInHz = D3DPRESENT_RATE_DEFAULT;
	p_Params.Flags = D3DCREATE_MULTITHREADED;

	/*if (FAILED(p_Object->CreateDeviceEx(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd, D3DCREATE_HARDWARE_VERTEXPROCESSING, &p_Params, 0, &p_Device)))
	{
		exit(1);
	}
	else
	{
		p_Object->Release();
		UnregisterClass(_T("XGui"), hInstance);
	}
	*/

	if (pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd, D3DCREATE_HARDWARE_VERTEXPROCESSING, &p_Params, &p_Device) < 0)
	{
		pD3D->Release();
		UnregisterClass(wcharenc("XGui"), hInstance);
	}

	if (!p_Line)
		D3DXCreateLine(p_Device, &p_Line);
	else
		p_Line->SetAntialias(1);
	D3DXCreateFont(p_Device, 18, 0, 0, 0, false, DEFAULT_CHARSET, OUT_CHARACTER_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH, encstr(u8"Microsoft Yahei"), &pFontSmall);
	D3DXCreateFont(p_Device, 17, 0, 0, 0, false, DEFAULT_CHARSET, OUT_CHARACTER_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH, encstr(u8"Microsoft Yahei"), &pFontBigGBK);
	D3DXCreateFontA(p_Device, 17, 0, 0, 0, false, DEFAULT_CHARSET, OUT_CHARACTER_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH, charenc("Microsoft Yahei"), &pu8Font);
	D3DXCreateFont(p_Device, 28, 0, 0, 0, false, DEFAULT_CHARSET, OUT_CHARACTER_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH, encstr(u8"Microsoft Yahei"), &pFontBig);
	D3DXCreateFont(p_Device, 32, 0, 0, 0, false, DEFAULT_CHARSET, OUT_CHARACTER_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH, encstr(u8"Microsoft Yahei"), &pFontSuperBig);
	D3DXCreateFontA(p_Device, 28, 0, 0, 0, false, DEFAULT_CHARSET, OUT_CHARACTER_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH, charenc("Microsoft Yahei"), &pFontBigUTF);
	D3DXCreateFontA(p_Device, 32, 0, 0, 0, false, DEFAULT_CHARSET, OUT_CHARACTER_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH, charenc("Microsoft Yahei"), &pFontSuperBigUTF);
	ImGui_ImplDX9_Init(hWnd, p_Device);
	//A.SecCheck(); // :-)
	static bool huoji = false;
	if (!huoji)
	{
		ImGui_ImplDX9_Init(D::hWnd, p_Device);
		ImGuiStyle& style = ImGui::GetStyle();
		style.Colors[ImGuiCol_Text] = ImVec4(0.78f, 0.78f, 0.78f, 1.00f);
		style.Colors[ImGuiCol_TextDisabled] = ImVec4(0.59f, 0.59f, 0.59f, 1.00f);
		style.Colors[ImGuiCol_WindowBg] = ImVec4(0.13f, 0.13f, 0.13f, 1.00f);
		style.Colors[ImGuiCol_ChildWindowBg] = ImVec4(0.11f, 0.11f, 0.11f, 1.00f);
		style.Colors[ImGuiCol_Border] = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
		style.Colors[ImGuiCol_FrameBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.09f);
		style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.39f, 0.39f, 0.39f, 1.00f);
		style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.04f, 0.04f, 0.04f, 0.88f);
		style.Colors[ImGuiCol_TitleBg] = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
		style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.00f, 0.00f, 0.00f, 0.20f);
		style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
		style.Colors[ImGuiCol_MenuBarBg] = ImVec4(0.35f, 0.35f, 0.35f, 1.00f);
		style.Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.13f, 0.13f, 0.13f, 1.00f);
		style.Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.24f, 0.40f, 0.95f, 1.00f);
		style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.24f, 0.40f, 0.95f, 0.59f);
		style.Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
		style.Colors[ImGuiCol_SliderGrab] = ImVec4(1.00f, 1.00f, 1.00f, 0.59f);
		style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.24f, 0.40f, 0.95f, 1.00f);
		style.Colors[ImGuiCol_Button] = ImVec4(0.24f, 0.40f, 0.95f, 1.00f);
		style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.24f, 0.40f, 0.95f, 0.59f);
		style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.39f, 0.39f, 0.39f, 1.00f);
		style.Colors[ImGuiCol_Header] = ImVec4(0.24f, 0.40f, 0.95f, 1.00f);
		style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.24f, 0.40f, 0.95f, 0.59f);
		style.Colors[ImGuiCol_HeaderActive] = ImVec4(0.39f, 0.39f, 0.39f, 1.00f);
		style.Colors[ImGuiCol_ColumnHovered] = ImVec4(0.70f, 0.02f, 0.60f, 0.22f);
		style.Colors[ImGuiCol_CloseButton] = ImVec4(0.24f, 0.40f, 0.95f, 1.00f);
		style.Colors[ImGuiCol_CloseButtonHovered] = ImVec4(0.24f, 0.40f, 0.95f, 0.59f);
		style.WindowPadding = ImVec2(15, 15);
		style.FramePadding = ImVec2(5, 5);
		style.ItemSpacing = ImVec2(12, 8);
		style.ItemInnerSpacing = ImVec2(8, 6);
		style.IndentSpacing = 23.0f;
		style.ScrollbarSize = 8.0f;
		style.ScrollbarRounding = 5.0f;
		style.GrabMinSize = 3.0f;
		style.GrabRounding = 3.0f;
		huoji = true;
	}
	if (p_Device) {
		p_Device->SetRenderState(D3DRS_ZENABLE, false);
		p_Device->SetRenderState(D3DRS_ALPHABLENDENABLE, true);
		p_Device->SetRenderState(D3DRS_SCISSORTESTENABLE, false);
		if (!Vars.Lazy.Flags)
			exit(1);
	}

	return 0;
}

bool NearPlayerDetectWarning = true;
void D::ExitHax() {
	ImGui_ImplDX9_Shutdown();
	if (p_Device) p_Device->Release();
	if (p_Object) p_Object->Release();
	if (p_Line) p_Line->Release();
	exit(1);
}
void AimBotTab()
{
	ImGui::Checkbox(charenc(jhzm), &Vars.Aimbot.AimbotEnable);
	//-----

	ImGui::Separator();
	ImGui::Columns(2, charenc(aimbotconfig), true);
	{
		ImGui::Combo(charenc(zmaj), &Vars.Aimbot.AimKey, keyNames, ARRAYSIZE(keyNames));
		ImGui::SliderFloat(charenc(fovSTRBYTE), &Vars.Aimbot.FOV, 1.f, 20.f);
		ImGui::SliderFloat(charenc(zdmzjl), &Vars.Aimbot.Dist, 1.f, 1000.f);
		ImGui::Checkbox(charenc(mzyj), &Vars.Aimbot.AimMother);
	}

	ImGui::NextColumn();
	{
		ImGui::Combo(charenc(zmbw), &Vars.Aimbot.Bone, AimBones, ARRAYSIZE(AimBones));
		ImGui::Checkbox(charenc(sjbw), &Vars.Aimbot.RandBone);
	}
	if (ImGui::Button(charenc(tc)))
		canExit = true;
	ImGui::Columns(1);
	ImGui::Separator();
}
void VisualTab()
{
	ImGui::Checkbox(charenc(jhesp), &Vars.Visual.VisualEnable);
	//----- 

	ImGui::Separator();
	ImGui::Columns(3, charenc(VisualConfig), true);
	{
		ImGui::Checkbox(charenc(xszx), &Vars.Visual.DrawCrosshair);
		ImGui::Checkbox(charenc(xsfk), &Vars.Visual.Box);
		ImGui::Checkbox(charenc(xsgg), &Vars.Visual.Ske);
		ImGui::Checkbox(charenc(xsxl), &Vars.Visual.Health);
		ImGui::Checkbox(charenc(xsdy), &Vars.Visual.isShowTeammate);
		ImGui::Checkbox(charenc(xszjdj), &Vars.Visual.isShowCar);
		ImGui::Checkbox(charenc(glzswp), &Vars.Visual.isFilter);
		ImGui::Checkbox(charenc(xsdwjkjsjg), &Vars.Visual.NearPlayerDetectWarning);

		ImGui::Checkbox(charenc("Ping"), &Vars.Visual.isShowPing);
	}

	ImGui::NextColumn();
	{
		ImGui::RadioButton(charenc(gbwzxx), &Vars.Visual.StrShow, 0);
		ImGui::RadioButton(charenc(xsszxljlxs), &Vars.Visual.StrShow, 1);
		ImGui::RadioButton(charenc(xsszxljlxsgdxx), &Vars.Visual.StrShow, 2);
		ImGui::SliderFloat(charenc(hsjgzdjl), &Vars.Visual.RedWarnDST, 1, Vars.Visual.YellowWarnDST);
		ImGui::SliderFloat(charenc(huangsejgzdjl), &Vars.Visual.YellowWarnDST, Vars.Visual.RedWarnDST, 1000);
		ImGui::SliderInt(charenc(zyxswp), &Vars.Visual.wespdt, 1, 1000);
		ImGui::SliderInt(charenc(zyxswj), &Vars.Visual.maxDST, 1, 1500);
	}
	ImGui::NextColumn();
	{
		ImGui::Checkbox(charenc(jhdjxs), &Vars.Visual.isShowItem);					// 激活道具显示
		ImGui::Checkbox(charenc(xswq), &Vars.Visual.isShowGun);							// 显示武器
		ImGui::Checkbox(charenc(显示弹药), &Vars.Visual.isShowAmmo);					// 显示弹药
		ImGui::Checkbox(charenc(武器配件), &Vars.Visual.isShowPart);							// 显示武器配件
		ImGui::Checkbox(charenc(显示药物), &Vars.Visual.isShowDrug);							// 显示药物
		ImGui::Checkbox(charenc(显示头盔防弹衣背包), &Vars.Visual.isShowEquip);						// 显示头盔/防弹衣/背包
		ImGui::Checkbox(charenc(显示其他), &Vars.Visual.isShowOther);						// 显示其他
	}
	ImGui::Columns(1);
	ImGui::Separator();

}

void MiscTab()
{
	ImGui::Checkbox(charenc(EnablesMisc), &Vars.Misc.MiscEnable);
	//-----

	ImGui::Separator();
	ImGui::Columns(2, charenc("MiscConfig"), true);
	{
		ImGui::Checkbox(charenc(KJWHD), &Vars.Misc.NoSway);
		ImGui::Checkbox(charenc(ZDWXZ), &Vars.Misc.NoGravity);
		ImGui::Checkbox(charenc(jhjs), &Vars.Misc.SpeedHack);
		//	ImGui::Checkbox(charenc(RageSpeedFuckUp), &Vars.Misc.RageSpeedHack);
	}
	ImGui::NextColumn();
	{
		ImGui::Checkbox(charenc(KSCYSZCY), &Vars.Misc.FastUseAndInWaterUser);
		ImGui::Checkbox(charenc(FastDuck), &Vars.Misc.FastProne);
		ImGui::SliderFloat(charenc(sudu), &Vars.Misc.MaxSpeed, 1.f, 720.f);
		//	ImGui::SliderInt(charenc(RageSpeedFuckUp), &Vars.Misc.FuckUp, 1, 10);
		//	ImGui::Checkbox(charenc(XGSH), &Vars.Misc.HitDamage);
	}

	ImGui::Columns(1);
	ImGui::Separator();

}
static ImColor mainColor = ImColor(54, 54, 54, 255);
void MainWindow()
{
	//ImGui::GetIO().MouseDrawCursor = Vars.Menu.Opened;
	ImGui_ImplDX9_NewFrame();
	static int page = 0;
	if (Vars.Menu.Opened)
	{
		ImGui::SetNextWindowSize(ImVec2(960, 645), ImGuiSetCond_FirstUseEver);
		if (ImGui::Begin(charenc(RookiezSTRBYTE), &Vars.Menu.Opened, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_ShowBorders))
		{
			const char* tabs[] = {
				zmsz,
				sjsz,
				magicstr
			};
			int tabs_size = sizeof(tabs) / sizeof(tabs[0]);

			for (int i = 0; i < tabs_size; i++)
			{
				ImVec2 windowSize = ImGui::GetWindowSize();
				int width = windowSize.x / tabs_size - 9;

				int distance;
				if (i == page)
					distance = 0;
				else
					distance = i > page ? i - page : page - i;
				ImGui::GetStyle().Colors[ImGuiCol_Button] = ImVec4(
					mainColor.Value.x - (distance * 0.035f),
					mainColor.Value.y - (distance * 0.035f),
					mainColor.Value.z - (distance * 0.035f),
					mainColor.Value.w
				);
				ImGui::GetStyle().Colors[ImGuiCol_Button] = mainColor;

				if (ImGui::Button(XorString(tabs[i]), ImVec2(width, 0)))
					page = i;
				if (i < tabs_size - 1)
					ImGui::SameLine();
			}

			switch (page)
			{
			case 0:
				AimBotTab();
				break;
			case 1:
				VisualTab();
				break;
			case 2:
				MiscTab();
				break;
			}
			ImGui::End();
		}
	}


	ImGui::Render();
}
#pragma region 3DVec3
void  D::GetAxes(D3DXVECTOR3 Rot, D3DXVECTOR3 &X, D3DXVECTOR3 &Y, D3DXVECTOR3 &Z) {
	//Pitch Roll Yaw -> x z y
	float	M[4][4];

	const float	SR = sin(Rot.z * PI / 180.f);
	const float	SP = sin(Rot.x * PI / 180.f);
	const float	SY = sin(Rot.y * PI / 180.f);
	const float	CR = cos(Rot.z * PI / 180.f);
	const float	CP = cos(Rot.x * PI / 180.f);
	const float	CY = cos(Rot.y * PI / 180.f);

	M[0][0] = CP * CY;
	M[0][1] = CP * SY;
	M[0][2] = SP;
	M[0][3] = 0.f;

	M[1][0] = SR * SP * CY - CR * SY;
	M[1][1] = SR * SP * SY + CR * CY;
	M[1][2] = -SR * CP;
	M[1][3] = 0.f;

	M[2][0] = -(CR * SP * CY + SR * SY);
	M[2][1] = CY * SR - CR * SP * SY;
	M[2][2] = CR * CP;
	M[2][3] = 0.f;

	M[3][0] = 0.0f;
	M[3][1] = 0.0f;
	M[3][2] = 0.0f;
	M[3][3] = 1.f;

	X.x = M[0][0]; X.y = M[0][1]; X.z = M[0][2];
	Y.x = M[1][0]; Y.y = M[1][1]; Y.z = M[1][2];
	Z.x = M[2][0]; Z.y = M[2][1]; Z.z = M[2][2];
}

bool D::toScreen(D3DXVECTOR3 rotat, D3DXVECTOR3 location, float fov, D3DXVECTOR3 in, D3DXVECTOR2& screenPosition) {
	D3DXVECTOR3 AxisX, AxisY, AxisZ, Delta, Transformed;

	GetAxes(rotat, AxisX, AxisY, AxisZ);
	Delta = in - location;

	Transformed.x = D3DXVec3Dot(&Delta, &AxisY);
	Transformed.y = D3DXVec3Dot(&Delta, &AxisZ);
	Transformed.z = D3DXVec3Dot(&Delta, &AxisX);

	if (Transformed.z < 1.00f)
		return false;

	auto CENTERX = (Width / 2) / 2.f;
	auto CENTERY = (Height / 2) / 2.f;

	screenPosition.x = CENTERX + Transformed.x * (CENTERX / tanf(fov * PI / 360.0f)) / Transformed.z;
	screenPosition.y = CENTERY - Transformed.y * (CENTERX / tanf(fov * PI / 360.0f)) / Transformed.z;

	return true;
}
#pragma endregion
//---------------------- Math
#pragma region Math
#define M_PI		3.14159265358979323846f
#define DEG2RAD( a ) ( ( (a) * M_PI ) / 180.0F )
#define M_PI_F		((float)(M_PI))	
#define RAD2DEG( x  )  ( (float)(x) * (float)(180.f / M_PI_F) )

void inline SinCos(float radians, float *sine, float *cosine)
{
	*sine = sin(radians);
	*cosine = cos(radians);
}

void AngleVectors(const D3DXVECTOR3 &angles, D3DXVECTOR3 *forward)
{
	float sp, sy, cp, cy;
	SinCos(DEG2RAD(angles[1]), &sy, &cy);
	SinCos(DEG2RAD(angles[0]), &sp, &cp);
	forward->x = cp*cy;
	forward->y = cp*sy;
	forward->z = -sp;

}

float GetFov(const D3DXVECTOR3& viewAngle, const D3DXVECTOR3& aimAngle)
{
	D3DXVECTOR3 ang, aim;
	AngleVectors(viewAngle, &aim);
	AngleVectors(aimAngle, &ang);
	float tempDoc = (ang.x*aim.x + ang.y*aim.y + ang.z*aim.z);
	return RAD2DEG(acos(tempDoc / (aim.x*aim.x + aim.y*aim.y + aim.z*aim.z))); // FUCK OFF

}
void ClampAngles(D3DXVECTOR3 &input)
{
	while (input.y < -180.0f)
		input.y += 360.0f;
	while (input.y > 180.0f)
		input.y -= 360.0f;

	if (input.x < -74.f) //The actual value for max pitch seems to be 74.9999 something, but 74 will work fine.
		input.x = -74.f;
	if (input.x > 74.f)
		input.x = 74.f;

}
D3DXVECTOR3 CalcAngle(D3DXVECTOR3 LocalHeadPosition, D3DXVECTOR3 AimPosition)
{
	D3DXVECTOR3 vecDelta = D3DXVECTOR3((LocalHeadPosition.x - AimPosition.x), (LocalHeadPosition.y - AimPosition.y), (LocalHeadPosition.z - AimPosition.z));
	float hyp = sqrt(vecDelta.x * vecDelta.x + vecDelta.y * vecDelta.y);

	D3DXVECTOR3 ViewAngles;
	ViewAngles.x = -atan(vecDelta.z / hyp) * (float)(180.f / PI);
	ViewAngles.y = atan(vecDelta.y / vecDelta.x) * (float)(180.f / PI);
	ViewAngles.z = 0.f;

	if (vecDelta.x >= 0.f)
		ViewAngles.y += 180.0f;

	return ViewAngles;
}
#pragma endregion
//---------------------- Math
D3DXVECTOR3 vecLocalEyeLocation;
D3DXVECTOR3 BestangDelta;

bool FoundBestAim = false;
float flFov = 0;
float BestDist = 0;
D3DXVECTOR3 DuckaimBone;
int rendermaxplayer = 0, nearplayer = 0, sonearplayer = 0;

void D::zRender() { //Render
	isRendering = true;
	p_Device->Clear(0, 0, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0, 1.0f, 0);
	p_Device->BeginScene();
	D::Render();
	p_Device->EndScene();
	p_Device->Present(NULL, NULL, NULL, NULL);
	if (canExit) D::ExitHax();
	isRendering = false;
}
void D::AimBot()
{
	if (!LocalPlayer || !LocalPlayerController || !(GetAsyncKeyState(Vars.Aimbot.AimKey) & 0x8000) || !Vars.Aimbot.AimbotEnable)
		return;
	isAiming = true;
	bool isTeamMate = false;
	//Aimbot FOV
	flFov = Vars.Aimbot.FOV;
	BestDist = Vars.Aimbot.Dist;
	FoundBestAim = false;
	for (int nIndex = 0; nIndex < EntityCount; nIndex++)
	{
		isTeamMate = false;
		//CamMag Offset
		QWORD Actor = Mem::Read<QWORD>(EntityList + nIndex * 0x8); //0x8
		QWORD pLocal = Mem::Read<QWORD>(LocalPlayerController + 0x3A8);
		if (!Actor || !pLocal || pLocal == Actor) continue;

		DWORD_PTR PlayerootComponent = Mem::Read<DWORD_PTR>(Actor + 0x180);

		DWORD_PTR mesh = Mem::Read<DWORD_PTR>(Actor + 0x400);

		D3DXVECTOR3 vecPlayerLocation = Mem::Read<D3DXVECTOR3>(PlayerootComponent + 0x1A0);

		D3DXVECTOR3 vecRelativePos = LocalPos - vecPlayerLocation;
		int lDeltaInMeters = sqrtf(vecRelativePos.x * vecRelativePos.x + vecRelativePos.y * vecRelativePos.y + vecRelativePos.z * vecRelativePos.z) / 100;

		QWORD APawn = Mem::Read<QWORD>(Actor + 0x168);
		QWORD localAPawn = Mem::Read<QWORD>(pLocal + 0x168);
		DWORD_PTR localPlayerState = Mem::Read<DWORD_PTR>(localAPawn + 0x3C0);
		DWORD_PTR PlayerState = Mem::Read<DWORD_PTR>(APawn + 0x3C0);

		int PlayerTeamNum = Mem::Read<int>(PlayerState + 0x444);
		int localPlayerTeamNum = Mem::Read<int>(localPlayerState + 0x444);

		if (localPlayerTeamNum == PlayerTeamNum)
			isTeamMate = true;

		if (Vars.Aimbot.AimbotEnable && (GetAsyncKeyState(Vars.Aimbot.AimKey) & 0x8000))
		{
			if (!Vars.Aimbot.RandBone)
			{
				if (Vars.Aimbot.Bone == 0)
					DuckaimBone = GetBoneWithRotation(mesh, Bones::Head);
				else if (Vars.Aimbot.Bone == 1)
					DuckaimBone = GetBoneWithRotation(mesh, Bones::neck_01);
				else if (Vars.Aimbot.Bone == 2)
					DuckaimBone = GetBoneWithRotation(mesh, Bones::pelvis);
				else
					DuckaimBone = GetBoneWithRotation(mesh, Bones::Head);
			}
			else
			{
				static int RandBones[] = { 0, 1, 2 };
				int AimBot = static_cast<int>(RandBones[rand() % 3]);
				if (AimBot == 0)
					DuckaimBone = GetBoneWithRotation(mesh, Bones::Head);
				else if (AimBot == 1)
					DuckaimBone = GetBoneWithRotation(mesh, Bones::neck_01);
				else if (AimBot == 2)
					DuckaimBone = GetBoneWithRotation(mesh, Bones::pelvis);
				else
					DuckaimBone = GetBoneWithRotation(mesh, Bones::Head);
			}

			vecLocalEyeLocation = CameraCache.POV.Location;
			float fov = GetFov(Myangles, CalcAngle(vecLocalEyeLocation, DuckaimBone));
			if (Vars.Aimbot.AimMother) {
				if (fov < flFov &&  lDeltaInMeters < BestDist && lDeltaInMeters <= Vars.Aimbot.Dist)
				{
					flFov = fov;
					BestDist = lDeltaInMeters;
					BestangDelta = CalcAngle(vecLocalEyeLocation, DuckaimBone);
					ClampAngles(BestangDelta);
					FoundBestAim = true;
				}
			}
			else if (!isTeamMate) {
				if (fov < flFov  && lDeltaInMeters < BestDist && lDeltaInMeters <= Vars.Aimbot.Dist)
				{
					flFov = fov;
					BestDist = lDeltaInMeters;
					BestangDelta = CalcAngle(vecLocalEyeLocation, DuckaimBone);
					ClampAngles(BestangDelta);
					FoundBestAim = true;
				}
			}

			if (Vars.Aimbot.AimbotEnable && (GetAsyncKeyState(Vars.Aimbot.AimKey) & 0x8000) && FoundBestAim)
			{
				D3DXVECTOR3 temp = BestangDelta;
				ClampAngles(temp);
				Mem::Write<D3DXVECTOR3>(LocalPlayerController + 0x3D0, temp);
			}

		}
	}
	//Sleep(1);
	isAiming = false;
}
int savedmaxDST = 0;
bool NameFilter(gUchar name[64]) {
	if (!Vars.Visual.isFilter)
		return true;

	char *cname = WCharToChar(name->Text);
	if (strstr(cname, charenc(Txu)))
		return false;

	if (strstr(cname, charenc(ku)))
		return false;

	if (strstr(cname, charenc(paoxie)))
		return false;

	if (strstr(cname, charenc(zangbeixing)))
		return false;

	return true;
}

void D::Render()
{
#pragma region Render
	//text with shadow
	if (Vars.Menu.Opened)
		DrawShadowString(XorString("AA3 For PLAYERUNKNOWN'S BATTLEGROUNDS"), 5, 0, 240, 240, 250, pFontSmall);


	//	static char buf[255]; sprintf_s(buf, "%.3f ms %.3f FPS",1000.f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
//		DrawString(buf, 5, 25, 255, 255, 255, pFontSmall, true);
		//test renderFPS
	if (Vars.Visual.DrawCrosshair  && Vars.Visual.VisualEnable)
	{
		FillRGB((Width) / 4 - 22, Height / 4, 44, 1, 255, 255, 0, 255);
		FillRGB((Width) / 4, Height / 4 - 22, 1, 44, 255, 255, 0, 255);
	}
	//DwCrossHair
	MainWindow();
	//ImGui Render
	if (LocalPlayer && LocalPlayerController) {
		//Test for loc

		QWORD pLocal = Mem::Read<QWORD>(LocalPlayerController + 0x3A8);
		QWORD APawn = Mem::Read<QWORD>(LocalPlayerController + 0x418);
		QWORD CharacterMovement = Mem::Read<QWORD>(APawn + 0x0408); // 行动相关
		float LocalHP = Mem::Read<float>(APawn + 0x107C);
		if (LocalHP > 101.f || LocalHP < 0.f) return;
		if (Vars.Misc.MiscEnable)
		{
			int MovementMode = Mem::Read<int>(CharacterMovement + 0x01BC); //MovementMode

			if (Vars.Misc.SpeedHack)
			{
				if (MovementMode == MOVE_Walking)
				{
					Mem::Write<float>(CharacterMovement + 0x01E4, Vars.Misc.MaxSpeed); //MaxWalkSpeed
					Mem::Write<float>(CharacterMovement + 0x01E8, Vars.Misc.MaxSpeed); //MaxWalkSpeedCrouched	
				}
				if (MovementMode == MOVE_Swimming)
					Mem::Write<float>(CharacterMovement + 0x01EC, Vars.Misc.MaxSpeed); //MaxSwimSpeed
				if (MovementMode == MOVE_Flying)
					Mem::Write<float>(CharacterMovement + 0x01F0, Vars.Misc.MaxSpeed); //MaxFlySpeed
				if (MovementMode == MOVE_Custom)
					Mem::Write<float>(CharacterMovement + 0x01F4, Vars.Misc.MaxSpeed); //MaxCustomMovementSpeed
				/*
				Mem::Write<float>(APawn + 0x0DC0, Vars.Misc.MaxSpeed); //Prone_RunningSpeedModifier
				Mem::Write<float>(APawn + 0x0DC4, Vars.Misc.MaxSpeed); //Prone_SprintingSpeedModifier

				Mem::Write<float>(APawn + 0x0DA4, Vars.Misc.MaxSpeed); //Stand_RunningSpeedModifier
				Mem::Write<float>(APawn + 0x0DA8, Vars.Misc.MaxSpeed); //Stand_SprintingSpeedModifier

				Mem::Write<float>(APawn + 0x0DAC, Vars.Misc.MaxSpeed); //Stand_SprintingBigGunModifier
				Mem::Write<float>(APawn + 0x0DB0, Vars.Misc.MaxSpeed); //Stand_SprintingRifleModifier

				Mem::Write<float>(APawn + 0x0DB4, Vars.Misc.MaxSpeed); //Stand_SprintingSmallGunMOdifier
				Mem::Write<float>(APawn + 0x0DB8, Vars.Misc.MaxSpeed); //Crouch_RunningSpeedModifier

				Mem::Write<float>(APawn + 0x0DBC, Vars.Misc.MaxSpeed); //Crouch_SprintingSpeedModifier
				Mem::Write<float>(APawn + 0x0DC0, Vars.Misc.MaxSpeed); //Prone_RunningSpeedModifier
				Mem::Write<float>(APawn + 0x0DC4, Vars.Misc.MaxSpeed); //Prone_SprintingSpeedModifier
				*/
			}
			if (Vars.Misc.FastProne)
			{
				Mem::Write<float>(APawn + 0x0DC0, 9.f); //Prone_RunningSpeedModifier
				Mem::Write<float>(APawn + 0x0DC4, 9.f); //Prone_SprintingSpeedModifier
			}
			QWORD WeaponProcessor = Mem::Read<QWORD>(APawn + 0x09E8);
			if (WeaponProcessor)
			{
				int wIndex = Mem::Read<int>(WeaponProcessor + 0x0448);
				if (wIndex != -1) {
					QWORD nowWeapon = Mem::Read<QWORD>(Mem::Read<QWORD>(WeaponProcessor + 0x0438) + wIndex * 0x8); // 当前的武器
					QWORD WeaponConfig = nowWeapon + 0x0538;
					QWORD WeaponGunConfig = nowWeapon + 0x0838;
					QWORD FTrajectoryWeaponData = nowWeapon + 0x0A48;
					QWORD RecoilInfo = nowWeapon + 0x0AA8;

					if (Vars.Misc.NoSway && WeaponConfig) {
						// 开镜不摇
						Mem::Write<float>(WeaponConfig + 0x34, 0);
						Mem::Write<float>(WeaponConfig + 0x38, 0);
						Mem::Write<float>(WeaponConfig + 0x3C, 0);
						Mem::Write<float>(WeaponConfig + 0x40, 0);
						Mem::Write<float>(WeaponConfig + 0x44, 0);
						Mem::Write<float>(WeaponConfig + 0x48, 0);
					}

					if (Vars.Misc.NoGravity && FTrajectoryWeaponData && nowWeapon && RecoilInfo && WeaponConfig)
					{
						Mem::Write<float>(FTrajectoryWeaponData + 0x2C, 2147483647.0f);// InitialSpeed = FLT_MAX
						Mem::Write<float>(nowWeapon + 0x9F8, 0);// 无下落

						Mem::Write<float>(FTrajectoryWeaponData + 0x28, 0); //RecoilPatternScale
						Mem::Write<float>(FTrajectoryWeaponData + 0x24, 0); //RecoilRecoverySpeed
						Mem::Write<float>(FTrajectoryWeaponData + 0x20, 0); //RecoilSpeed

																			// RECOIL INFO
						Mem::Write<float>(RecoilInfo + 0x0000, 0); //VerticalRecoilMin
						Mem::Write<float>(RecoilInfo + 0x0004, 0); //VerticalRecoilMax

						Mem::Write<float>(RecoilInfo + 0x0048, 0); //RecoilValue_Climb
						Mem::Write<float>(RecoilInfo + 0x004C, 0); //RecoilValue_Fall
						Mem::Write<float>(RecoilInfo + 0x0050, 0); //RecoilModifier_Stand
						Mem::Write<float>(RecoilInfo + 0x0054, 0); //RecoilModifier_Crouch
						Mem::Write<float>(RecoilInfo + 0x0058, 0); //RecoilModifier_Prone
						Mem::Write<float>(RecoilInfo + 0x0040, 0); //RecoilSpeed_Horizontal
						Mem::Write<float>(RecoilInfo + 0x003C, 0); //RecoilSpeed_Vertical
						Mem::Write<float>(RecoilInfo + 0x0044, 0); //RecoverySpeed_Vertical
						Mem::Write<float>(RecoilInfo + 0x000C, 0); //VerticalRecoveryModifier

					}
					//			if (Vars.Misc.HitDamage && FTrajectoryWeaponData)
					//				Mem::Write<int>(FTrajectoryWeaponData + 0x30, 2147483647);// HitDamage = 2147483647
				}

			}


			QWORD InventoryFacade = Mem::Read<QWORD>(APawn + 0x09E0);
			QWORD Inventory = Mem::Read<QWORD>(InventoryFacade + 0x03B8);
			QWORD Equipment = Mem::Read<QWORD>(InventoryFacade + 0x03C0);
			QWORD ItemList = Mem::Read<QWORD>(Inventory + 0x03E8);
			int ItemCount = (int)Mem::Read<QWORD>(Inventory + 0x03F0);
			if (Vars.Misc.FastUseAndInWaterUser) {
				for (int n = 0; n < ItemCount; n++) {
					gUchar szName[64];
					gUchar szTag[64];

					QWORD dwItemPtr = Mem::Read<QWORD>(ItemList + n * 0x8);
					if (dwItemPtr == 0)
						continue;

					DWORD_PTR dwNamePtr = Mem::Read<DWORD_PTR>(Mem::Read<DWORD_PTR>(Mem::Read<DWORD_PTR>(dwItemPtr + 0x40) + 0x8));
					if (dwNamePtr != 0)
						*szName = Mem::Read<gUchar>(dwNamePtr);

					DWORD_PTR dwTagPtr = Mem::Read<DWORD_PTR>(Mem::Read<DWORD_PTR>(Mem::Read<DWORD_PTR>(dwItemPtr + 0x58) + 0x8));
					if (dwTagPtr != 0)
						*szTag = Mem::Read<gUchar>(dwTagPtr);

					if (!lstrcmp((LPCWSTR)szTag, L"能量物品") || !lstrcmp((LPCWSTR)szTag, L"恢复")) {
						Mem::Write<float>(dwItemPtr + 0x188 + 0x18, 0.001f);
						Mem::Write<byte>(dwItemPtr + 0x1B8, 1);
					}
				}
			}
		}

		//Vector LocalEyePos = Mem::Read<Vector>(LocalrootComponent + 0x1A0);
		//	LocalEyePos.z += Mem::Read<float>(LocalPlayer + 0x394);
		//char buf[255]; sprintf_s(buf, "\nPlayer Pos: X: %f Y: %f Z: %f", LocalPos.x, LocalPos.y, LocalPos.z);
		//DrawString(buf, 5, 25, 255, 255, 255, pFontSmall, true);

		rendermaxplayer = 0, nearplayer = 0, sonearplayer = 0;

		//Test for ang
		//	char bufz[255]; sprintf_s(bufz, "Player Ang: X: %f Y: %f Z: %f", Myangles.x, Myangles.y, Myangles.z);
		//	DrawString(bufz, 5, 45, 255, 255, 255, pFontSmall, false);

			//loopy ent
		bool isTeamMate = false;
		for (int nIndex = 0; nIndex < EntityCount; nIndex++)
		{
			//CamMag Offset
			QWORD Actor = Mem::Read<QWORD>(EntityList + nIndex * 0x8); //0x8
			QWORD pLocal = Mem::Read<QWORD>(LocalPlayerController + 0x3A8);
			if (!Actor || !pLocal || pLocal == Actor)
				continue;
			DWORD_PTR PlayerootComponent = Mem::Read<DWORD_PTR>(Actor + 0x180);
			auto uPlayerootComponent = (uintptr_t)PlayerootComponent;

			DWORD_PTR mesh = Mem::Read<DWORD_PTR>(Actor + 0x400);
			auto umesh = (uintptr_t)mesh;

			D3DXVECTOR3 vecPlayerLocation = Mem::Read<D3DXVECTOR3>(uPlayerootComponent + 0x1A0);
			//D3DXVECTOR3 vecPlayerRotation = Mem::Read<D3DXVECTOR3>(uPlayerootComponent + 0x1EC); //ZanShiYonBuShang

			QWORD APawn = Mem::Read<QWORD>(Actor + 0x168);
			QWORD localAPawn = Mem::Read<QWORD>(pLocal + 0x168);

			auto uAPawn = (uintptr_t)APawn;

			DWORD_PTR localPlayerState = Mem::Read<DWORD_PTR>(localAPawn + 0x3C0);
			DWORD_PTR PlayerState = Mem::Read<DWORD_PTR>(uAPawn + 0x3C0);

			int IDnumber = Mem::Read<int>(Actor + 0x18); //Entity ID number

			float ActorHP = Mem::Read<float>(Actor + 0x107C); //Entity Health

			if (PlayerState != 0 && ActorHP > 0.f/*&& ((IDnumber >= 65277 && 65301 >= IDnumber) || (66832 <= IDnumber && IDnumber <= 66845)))*/
				&& (IDnumber == ActorIds[0] || IDnumber == ActorIds[1]
					|| IDnumber == ActorIds[2] || IDnumber == ActorIds[3]
					|| IDnumber == ActorIds[4] || IDnumber == ActorIds[5]))
			{

				if (Vars.Visual.isShowPing) {
					int iPing = Mem::Read<int>(localPlayerState + 0x03A4);
					char bufzz[32]; sprintf_s(bufzz, "Ping:%d", iPing);
					DrawString(bufzz, 5, 5, 255, 255, 255, pFontBig, true);
				}

				int PlayerTeamNum = Mem::Read<int>(PlayerState + 0x444);
				int localPlayerTeamNum = Mem::Read<int>(localPlayerState + 0x444);
				if (Vars.Visual.isShowTeammate) {
					if (localPlayerTeamNum == PlayerTeamNum)
						isTeamMate = true;
				}
				else if (localPlayerTeamNum == PlayerTeamNum)
					continue;

				D3DXVECTOR3 vecRelativePos = LocalPos - vecPlayerLocation;
				int lDeltaInMeters = sqrtf(vecRelativePos.x * vecRelativePos.x + vecRelativePos.y * vecRelativePos.y + vecRelativePos.z * vecRelativePos.z) / 100;

				if (lDeltaInMeters > Vars.Visual.maxDST && Vars.Visual.maxDST != 1500)
					continue;
				//无需等待W2S获取位置
				float color = 0;
				D3DXVECTOR3 clr;
				//if (lDeltaInMeters > 400.f)
				//	continue;
				if (isTeamMate) {
					color = D3DCOLOR_RGBA(0, 200, 255, 255); //be blue
					clr = D3DXVECTOR3(0, 255, 255);
				}
				else { // if not teammate
					if (lDeltaInMeters <= Vars.Visual.RedWarnDST) {
						color = D3DCOLOR_RGBA(255, 0, 0, 255); //color red, if less than 150m
						clr = D3DXVECTOR3(255, 0, 0);
						sonearplayer++;
					}
					else if (lDeltaInMeters > Vars.Visual.RedWarnDST && lDeltaInMeters <= Vars.Visual.YellowWarnDST) {
						color = D3DCOLOR_RGBA(255, 255, 0, 255); //color yellow, if less than 300m and greater than 150m
						clr = D3DXVECTOR3(255, 255, 0);
						nearplayer++;
					}
					else {
						color = D3DCOLOR_RGBA(0, 255, 0, 255); //color green, if greater than 300m
						clr = D3DXVECTOR3(0, 255, 0);
					}
				}
				/*
				if (Vars.Misc.TelePort && Vars.Misc.MiscEnable)
				{
					D3DXVECTOR3 location = vecPlayerLocation;

					location.x += 5;
					location.y += 5;
					location.z += 5;
					if(lDeltaInMeters <= Vars.Misc.TelePortDuck)
						Mem::Write<D3DXVECTOR3>(uPlayerootComponent + 0x1A0, location);
				}*/


				D3DXVECTOR2 Output2D, sHead, sOrig;
				if (toScreen(CameraCache.POV.Rotation, CameraCache.POV.Location, CameraCache.POV.FOV, vecPlayerLocation, Output2D)
					&& toScreen(CameraCache.POV.Rotation, CameraCache.POV.Location, CameraCache.POV.FOV, GetBoneWithRotation(mesh, Bones::foot_l), sOrig)
					&& toScreen(CameraCache.POV.Rotation, CameraCache.POV.Location, CameraCache.POV.FOV, GetBoneWithRotation(mesh, Bones::forehead), sHead)) {


					float Height = sOrig.y - sHead.y;
					float Width = Height * 0.6;

					if (Vars.Visual.Box && Vars.Visual.VisualEnable)
						DrawBox(sHead.x - (Width / 2.f), sHead.y, Width, Height, color, 2);

					if (Vars.Visual.VisualEnable)
					{
						switch (Vars.Visual.StrShow) {
							WCHAR wbf[256];
						case 0:
							break;
						case 1:
							//WCHAR PlayerName = Mem::Read<WCHAR>(PlayerState + 0x3A8);//拿不到
							wsprintfW(wbf, encstr(xljl)
								, (int)ActorHP
								, (int)lDeltaInMeters);
							WDstr(wbf, sHead.x + (Width / 2.f), sHead.y, clr.x, clr.y, clr.z, pu8Font);
							break;
						case 2:
							FTslPlayerScores PlayerScores = Mem::Read<FTslPlayerScores>(PlayerState + 0x448);
							wsprintf(wbf, encstr(xljlfsjsfs)
								, (int)ActorHP
								, (int)lDeltaInMeters
								, (int)PlayerScores.ScoreByRanking * 1000
								, (int)PlayerScores.ScoreByKill * 1000);
							WDstr(wbf, sHead.x + (Width / 2.f), sHead.y, clr.x, clr.y, clr.z, pu8Font);
							break;
						}
					}

					if (Vars.Visual.Health && Vars.Visual.VisualEnable) {
						int health = ActorHP;
						int m_H = Height;
						int m_W = Width;
						int m_X = sHead.x - (Width / 2.f);
						int m_Y = sHead.y;
						UINT hp = m_H - (UINT)((m_H * health) / 100);
						int Red = 255 - (health*2.55);
						int Green = health*2.55;
						FillRGB(sHead.x - (Width / 2.f), sHead.y, Width / 10, Height - hp, Red, Green, 0, 255);
					}

					if (Vars.Visual.Ske && Vars.Visual.VisualEnable) {
						if (mesh && lDeltaInMeters < 110.f)
							DrawSkeleton(CameraCache.POV.Rotation, CameraCache.POV.Location, CameraCache.POV.FOV, mesh, color); //draw skeleton, is distance is less than equal to 110m
					}
					/*
					if ((int)lDeltaInMeters > 2) {
						Output2D.y -= 2;
						FStrings name = Mem::Read<FStrings>(playerstate + 0x3A8);
						//char a[128]; jksn_utf16_to_utf8((const uint16_t *)name.wName, a, name.MaxCount);
						char bufzz[255]; sprintf_s(bufzz, ",距离:%dm,生命：%dHP",(int)lDeltaInMeters, (int)ActorHP);
						DrawString("O", Output2D.x, Output2D.y, 255, 0, 0, pFontSmall);
						DrawString(bufzz, Output2D.x, Output2D.y - 15, 255, 0, 0, pFontSmall);
					}*/
				}
				//D3DXVECTOR3 PlayerPos = Mem::Read<D3DXVECTOR3>(uPlayerootComponent + 0x1A0);
				//if (WorldToScreen(PlayerPos, Output2D) == true)
				//{
				//	char buf2[255]; sprintf_s(buf2, "CR: X: %f Y: %f", Output2D.x, Output2D.y);
				//	DrawString(buf2, 5, 125, 240, 240, 250, pFontSmall);
				//	DrawString("O",Output2D.x, Output2D.y,255,0,0, pFontSmall);
				//}
				rendermaxplayer++;
			}
			if (Vars.Visual.VisualEnable) {
				if (Vars.Visual.isShowCar) {
					string entityName = GetNameFromId(IDnumber);
					for (uint32_t i = 0; i < names.size(); i++) {
						if (names[i].searchname == entityName) {
							D3DXVECTOR2 Output2D;
							if (toScreen(CameraCache.POV.Rotation, CameraCache.POV.Location, CameraCache.POV.FOV, vecPlayerLocation, Output2D)) {
								D3DXVECTOR3 vecRelativePos = LocalPos - vecPlayerLocation;
								int lDeltaInMeters = sqrtf(vecRelativePos.x * vecRelativePos.x + vecRelativePos.y * vecRelativePos.y + vecRelativePos.z * vecRelativePos.z) / 100;
								if (Vars.Visual.wespdt == 1000 || Vars.Visual.wespdt >= (int)lDeltaInMeters) {
									//char bufzz[255]; sprintf_s(bufzz, "%s[%dM]", names[i].outputname.c_str(), (int)lDeltaInMeters);
									//DrawString(bufzz, Output2D.x, Output2D.y, 0, 235, 255, pFontSmall, false);
									WCHAR wbf[256]; wsprintfW(wbf, encstr(showcar), names[i].outputname.c_str(), (int)lDeltaInMeters);
									WDstr(wbf, Output2D.x, Output2D.y, 0, 235, 255, pu8Font);
								}
							}
						}
					}
				}
				if (IDnumber == 7263 && Vars.Visual.isShowItem) { //DroppedItemGroup

					DWORD_PTR ItemGroupStart = Mem::Read<DWORD_PTR>(Actor + 0x2D8);
					auto uItemGroupStart = (uintptr_t)ItemGroupStart;

					UINT32 ItemGroupCount = Mem::Read<DWORD_PTR>(Actor + 0x2E0);
					auto uItemGroupCount = (uintptr_t)ItemGroupCount;
					for (int n = 0; n < uItemGroupCount; n++) {
						D3DXVECTOR3 vItemPos;
						gUchar szName[64];
						gUchar szTag[64];
						DWORD_PTR dwItemPtr = Mem::Read<DWORD_PTR>(uItemGroupStart + n * 0x10);
						if (dwItemPtr == 0)
							continue;
						UINT32 ItemGroupID = Mem::Read<DWORD_PTR>(dwItemPtr + 0x18);
						if (ItemGroupID != 7921)
							continue;

						DWORD_PTR dwItemCoord = Mem::Read<DWORD_PTR>(dwItemPtr + 0x458);
						if (dwItemCoord != 0)
							vItemPos = Mem::Read<D3DXVECTOR3>(dwItemCoord + 0x174);

						DWORD_PTR dwTagPtr = Mem::Read<DWORD_PTR>(Mem::Read<DWORD_PTR>(Mem::Read<DWORD_PTR>(Mem::Read<DWORD_PTR>(dwItemPtr + 0x448) + 0x58) + 0x8));
						if (dwTagPtr != 0)
							*szTag = Mem::Read<gUchar>(dwTagPtr);

						bool allowShow = Vars.Visual.isShowOther;
						D3DXVECTOR3 xcolor = D3DXVECTOR3(255, 105, 105);
						if (!lstrcmp((LPCWSTR)szTag, L"主武器") || !lstrcmp((LPCWSTR)szTag, L"手枪")) {
							allowShow = Vars.Visual.isShowGun;
							xcolor = D3DXVECTOR3(228, 54, 17);
						}
						if (!lstrcmp((LPCWSTR)szTag, L"弹药")) {
							allowShow = Vars.Visual.isShowAmmo;
							xcolor = D3DXVECTOR3(250, 190, 0);
						}

						if (!lstrcmp((LPCWSTR)szTag, L"配件")) {
							allowShow = Vars.Visual.isShowPart;
							xcolor = D3DXVECTOR3(229, 250, 0);
						}
						if (!lstrcmp((LPCWSTR)szTag, L"能量物品") || !lstrcmp((LPCWSTR)szTag, L"恢复")) {
							allowShow = Vars.Visual.isShowDrug;
							xcolor = D3DXVECTOR3(0, 236, 255);
						}

						if (!lstrcmp((LPCWSTR)szTag, L"头饰") || !lstrcmp((LPCWSTR)szTag, L"防弹衣") || !lstrcmp((LPCWSTR)szTag, L"背包")) {
							allowShow = Vars.Visual.isShowEquip;
							xcolor = D3DXVECTOR3(182, 0, 255);
						}
						if (!allowShow) continue;

						DWORD_PTR dwNamePtr = Mem::Read<DWORD_PTR>(Mem::Read<DWORD_PTR>(Mem::Read<DWORD_PTR>(Mem::Read<DWORD_PTR>(dwItemPtr + 0x448) + 0x40) + 0x8));
						if (dwNamePtr != 0)
							*szName = Mem::Read<gUchar>(dwNamePtr);
						D3DXVECTOR2 Output2D;

						if (toScreen(CameraCache.POV.Rotation, CameraCache.POV.Location, CameraCache.POV.FOV, vItemPos, Output2D)) {
							D3DXVECTOR3 vecRelativePos = LocalPos - vItemPos;

							int lDeltaInMeters = sqrtf(vecRelativePos.x * vecRelativePos.x + vecRelativePos.y * vecRelativePos.y + vecRelativePos.z * vecRelativePos.z) / 100;
							if ((Vars.Visual.wespdt == 1000 || Vars.Visual.wespdt >= (int)lDeltaInMeters) && NameFilter(szName)) {
								WCHAR wbf[256]; wsprintfW(wbf, encstr(showcar), szName, (int)lDeltaInMeters);

								WDstr(wbf, Output2D.x, Output2D.y, xcolor.x, xcolor.y, xcolor.z, pu8Font);
							}
						}
					}

				}

			}
			//Loopy Done
		}

		//-NearDTC-
		if (Vars.Visual.VisualEnable && NearPlayerDetectWarning) {
			if (nearplayer >= 1) {
				WCHAR wbf[256]; wsprintfW(wbf, encstr(yellowDTCSTRBYTE), nearplayer);
				WDstr(wbf, cx / 2 - 125, cy / 2 + 50, 255, 255, 0, pFontBigUTF);
			}
			if (sonearplayer >= 1) {
				WCHAR wbf[256]; wsprintfW(wbf, encstr(redDTCSTRBYTE), sonearplayer);
				WDstr(wbf, cx / 2 - 150, cy / 2 + 120, 255, 0, 0, pFontSuperBigUTF);
			}
		}
		//-NearDTC-
	}

#pragma endregion
}
void D::FillRGB(float x, float y, float w, float h, int r, int g, int b, int a)
{
	D3DXVECTOR2 vLine[2];

	p_Line->SetWidth(w);

	vLine[0].x = x + w / 2;
	vLine[0].y = y;
	vLine[1].x = x + w / 2;
	vLine[1].y = y + h;

	p_Line->Begin();
	p_Line->Draw(vLine, 2, D3DCOLOR_RGBA(r, g, b, a));
	p_Line->End();
}
int D::DrawString(char* String, int x, int y, int r, int g, int b, ID3DXFont* ifont, bool isCenter = false)	//now not isCenter now is isShadow
{

	
	RECT ShadowPos;
	ShadowPos.left = x + 1;
	ShadowPos.top = y + 1;
	RECT FontPos;
	FontPos.left = x;
	FontPos.top = y;
	if (!isCenter) {
		//ifont->DrawTextA(0, String, strlen(String), &ShadowPos, DT_NOCLIP, D3DCOLOR_ARGB(255, r / 3, g / 3, b / 3));
		ifont->DrawTextA(0, String, strlen(String), &FontPos, DT_NOCLIP, D3DCOLOR_ARGB(255, r, g, b));
	}
	else {
		ifont->DrawTextA(0, String, strlen(String), &ShadowPos, DT_NOCLIP, D3DCOLOR_ARGB(255, r / 3, g / 3, b / 3));
		ifont->DrawTextA(0, String, strlen(String), &FontPos, DT_NOCLIP, D3DCOLOR_ARGB(255, r, g, b));
	}
	
	return 0;
}

void D::DrawLine(float x, float y, float xx, float yy, D3DCOLOR color)
{
	D3DXVECTOR2 dLine[2];

	p_Line->SetWidth(1.f);

	dLine[0].x = x;
	dLine[0].y = y;

	dLine[1].x = xx;
	dLine[1].y = yy;

	p_Line->Draw(dLine, 2, color);

}
int D::WDstr(wchar_t* zString, int x, int y, int r, int g, int b, ID3DXFont* ifont)
{
	RECT ShadowPos;
	ShadowPos.left = x + 1;
	ShadowPos.top = y + 1;
	RECT FontPos;
	FontPos.left = x;
	FontPos.top = y;
	ifont->DrawTextW(0, zString, wcslen(zString), &ShadowPos, DT_NOCLIP, D3DCOLOR_ARGB(255, r / 3, g / 3, b / 3));
	ifont->DrawTextW(0, zString, wcslen(zString), &FontPos, DT_NOCLIP, D3DCOLOR_ARGB(255, r, g, b));
	return 0;
}
void D::DrawBox(float x, float y, float width, float height, D3DCOLOR color)
{
	D3DXVECTOR2 points[5];
	points[0] = D3DXVECTOR2(x, y);
	points[1] = D3DXVECTOR2(x + width, y);
	points[2] = D3DXVECTOR2(x + width, y + height);
	points[3] = D3DXVECTOR2(x, y + height);
	points[4] = D3DXVECTOR2(x, y);
	p_Line->SetWidth(1);
	p_Line->Draw(points, 5, color);
}
void D::DrawBox(float x, float y, float width, float height, D3DCOLOR color, int wdt)
{
	D3DXVECTOR2 points[5];
	points[0] = D3DXVECTOR2(x, y);
	points[1] = D3DXVECTOR2(x + width, y);
	points[2] = D3DXVECTOR2(x + width, y + height);
	points[3] = D3DXVECTOR2(x, y + height);
	points[4] = D3DXVECTOR2(x, y);
	p_Line->SetWidth(wdt);
	p_Line->Draw(points, 5, color);
}
FTransform D::GetBoneIndex(DWORD_PTR mesh, int index)
{
	DWORD_PTR bonearray = Mem::Read<DWORD_PTR>(mesh + 0x790);
	if (bonearray)
		return Mem::Read<FTransform>(bonearray + (index * 0x30));
	FTransform fakertn;
	return fakertn;
}
D3DMATRIX D::MatrixMultiplication(D3DMATRIX pM1, D3DMATRIX pM2)
{
	D3DMATRIX pOut;
	pOut._11 = pM1._11 * pM2._11 + pM1._12 * pM2._21 + pM1._13 * pM2._31 + pM1._14 * pM2._41;
	pOut._12 = pM1._11 * pM2._12 + pM1._12 * pM2._22 + pM1._13 * pM2._32 + pM1._14 * pM2._42;
	pOut._13 = pM1._11 * pM2._13 + pM1._12 * pM2._23 + pM1._13 * pM2._33 + pM1._14 * pM2._43;
	pOut._14 = pM1._11 * pM2._14 + pM1._12 * pM2._24 + pM1._13 * pM2._34 + pM1._14 * pM2._44;
	pOut._21 = pM1._21 * pM2._11 + pM1._22 * pM2._21 + pM1._23 * pM2._31 + pM1._24 * pM2._41;
	pOut._22 = pM1._21 * pM2._12 + pM1._22 * pM2._22 + pM1._23 * pM2._32 + pM1._24 * pM2._42;
	pOut._23 = pM1._21 * pM2._13 + pM1._22 * pM2._23 + pM1._23 * pM2._33 + pM1._24 * pM2._43;
	pOut._24 = pM1._21 * pM2._14 + pM1._22 * pM2._24 + pM1._23 * pM2._34 + pM1._24 * pM2._44;
	pOut._31 = pM1._31 * pM2._11 + pM1._32 * pM2._21 + pM1._33 * pM2._31 + pM1._34 * pM2._41;
	pOut._32 = pM1._31 * pM2._12 + pM1._32 * pM2._22 + pM1._33 * pM2._32 + pM1._34 * pM2._42;
	pOut._33 = pM1._31 * pM2._13 + pM1._32 * pM2._23 + pM1._33 * pM2._33 + pM1._34 * pM2._43;
	pOut._34 = pM1._31 * pM2._14 + pM1._32 * pM2._24 + pM1._33 * pM2._34 + pM1._34 * pM2._44;
	pOut._41 = pM1._41 * pM2._11 + pM1._42 * pM2._21 + pM1._43 * pM2._31 + pM1._44 * pM2._41;
	pOut._42 = pM1._41 * pM2._12 + pM1._42 * pM2._22 + pM1._43 * pM2._32 + pM1._44 * pM2._42;
	pOut._43 = pM1._41 * pM2._13 + pM1._42 * pM2._23 + pM1._43 * pM2._33 + pM1._44 * pM2._43;
	pOut._44 = pM1._41 * pM2._14 + pM1._42 * pM2._24 + pM1._43 * pM2._34 + pM1._44 * pM2._44;

	return pOut;
}
D3DXVECTOR3 D::GetBoneWithRotation(DWORD_PTR mesh, int id)
{
	FTransform bone = GetBoneIndex(mesh, id);
	if (!bone.rot.y)
		return D3DXVECTOR3(-1, -1, -1);
	FTransform ComponentToWorld = Mem::Read<FTransform>(mesh + 0x190);
	D3DMATRIX Matrix;
	Matrix = MatrixMultiplication(bone.ToMatrixWithScale(), ComponentToWorld.ToMatrixWithScale());
	return D3DXVECTOR3(Matrix._41, Matrix._42, Matrix._43);
}
void D::DrawSkeleton(D3DXVECTOR3 rotat, D3DXVECTOR3 location, float fov, DWORD_PTR mesh, float color)
{
	D3DXVECTOR3 neckpos = GetBoneWithRotation(mesh, Bones::neck_01);
	D3DXVECTOR3 pelvispos = GetBoneWithRotation(mesh, Bones::pelvis);
	D3DXVECTOR3 previous(0, 0, 0);
	D3DXVECTOR3 current;
	D3DXVECTOR2 p1, c1;
	for (auto a : skeleton)
	{
		previous = D3DXVECTOR3(0, 0, 0);
		for (int bone : a)
		{
			current = bone == Bones::neck_01 ? neckpos : (bone == Bones::pelvis ? pelvispos : GetBoneWithRotation(mesh, bone));
			if (previous.x == 0.f)
			{
				previous = current;
				continue;
			}
			if (toScreen(rotat, location, fov, previous, p1) && toScreen(rotat, location, fov, current, c1)) {
				DrawLine(p1.x, p1.y, c1.x, c1.y, color);
				previous = current;
			}
		}
	}
}
int D::DrawShadowString(char* String, int x, int y, int r, int g, int b, ID3DXFont* ifont)
{

	RECT Font;
	Font.left = x;
	Font.top = y;
	RECT Fonts;
	Fonts.left = x + 1;
	Fonts.top = y;
	RECT Fonts1;
	Fonts1.left = x - 1;
	Fonts1.top = y;
	RECT Fonts2;
	Fonts2.left = x;
	Fonts2.top = y + 1;
	RECT Fonts3;
	Fonts3.left = x;
	Fonts3.top = y - 1;
	ifont->DrawTextA(0, String, strlen(String), &Fonts3, DT_NOCLIP, D3DCOLOR_ARGB(255, 1, 1, 1));
	ifont->DrawTextA(0, String, strlen(String), &Fonts2, DT_NOCLIP, D3DCOLOR_ARGB(255, 1, 1, 1));
	ifont->DrawTextA(0, String, strlen(String), &Fonts1, DT_NOCLIP, D3DCOLOR_ARGB(255, 1, 1, 1));
	ifont->DrawTextA(0, String, strlen(String), &Fonts, DT_NOCLIP, D3DCOLOR_ARGB(255, 1, 1, 1));
	ifont->DrawTextA(0, String, strlen(String), &Font, DT_NOCLIP, D3DCOLOR_ARGB(255, r, g, b));
	return 0;
}
