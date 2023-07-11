// The Long Dark Game Time StopWatch.cpp : Defines the entry point for the application.
//

#include "framework.h"
#include "StopWatch.h"
#include <string>
#include <sstream>
#include <chrono>
#include <fstream>
#include <assert.h>

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    SetHotKey(HWND, UINT, WPARAM, LPARAM);
void Timerproc(HWND unnamedParam1, UINT unnamedParam2, UINT_PTR unnamedParam3, DWORD unnamedParam4);
void RecalculateSplitStrings();

struct SavedState
{
	std::chrono::time_point<std::chrono::system_clock> timebegin;
	std::chrono::duration<double> timeduration;;
	std::chrono::duration<double> Previoustimeduration;
	INT64 SplitsTenths[4];
	INT64 AddedTime;
	INT intervalValue;
	float addminutes;
	float addhours;
	BOOL b_StartedState;
	BOOL b_Sound = true;
	BOOL b_TimeFormatDays;
};
INT64 GameMinutesTenths = {};
INT64 TimeSinceLastSplit = {};
SavedState LoadFromFile();
void SavetoFile(SavedState ss);

int g_window_width = 520;
int g_window_height = 330;
int buttonWidth = 120;
int buttonheight = 50;

float sixthx = g_window_width / 6.0f;
int x1 = int(sixthx - buttonWidth / 2.0f);
int x2 = int(sixthx * 3 - buttonWidth / 2.0f);
int x3 = int(sixthx * 5 - buttonWidth / 2.0f);

INT64 LastGameMinutes = 0;

HWND hMainWindow = nullptr;
HWND hwndStartButton = nullptr;
HWND hwndResetButton = nullptr;
HWND hwndSplitButton = nullptr;
HWND hwndInterval = nullptr;
HWND hWndLabel1 = nullptr;

HWND hWndAddminutesButton = nullptr;
HWND hWndAddHoursButton = nullptr;

HWND hWndAddminutes = nullptr;
HWND hWndAddHours = nullptr;

RECT redrawrect = {};

std::wstring TimerString;
std::wstring TimeSinceLastSplitString;
std::wstring SplitStrings[4];

HFONT myfont1 = nullptr;
HFONT splitfont = {};

SavedState state = LoadFromFile();

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	// Initialize global strings
	LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadStringW(hInstance, IDC_THELONGDARKSTOPWATCH, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// Perform application initialization:
	if (!InitInstance(hInstance, nCmdShow))
	{
		return FALSE;
	}

	HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_THELONGDARKSTOPWATCH));

	MSG msg;

	// Main message loop:
	while (GetMessage(&msg, nullptr, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return (int)msg.wParam;
}

ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEXW wcex = {};

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_THELONGDARKSTOPWATCH));
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_THELONGDARKSTOPWATCH);
	wcex.lpszClassName = szWindowClass;
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassExW(&wcex);
}

std::wstring GetAlarmLabel()
{
	std::wstring beepstring;
	if (state.intervalValue == 0)
	{
		beepstring = L"Beeps Disabled";
	}
	else if (state.intervalValue == 1)
	{
		beepstring = L"Beep every minute";
	}
	else
	{
		beepstring = L"Beep every " + std::to_wstring(state.intervalValue) + L" minutes";
	}
	return beepstring;
}

void SetFont()
{
	LOGFONTW font = { 0 };

	font.lfHeight = 30;
	font.lfWidth = 12;

	myfont1 = CreateFontIndirectW(&font);
	float scale = 0.75f;
	font.lfHeight = int(30 * scale);
	font.lfWidth = int(12 * scale);
	splitfont = CreateFontIndirectW(&font);
}

std::wstring StripZeroes(float value)
{
	std::wstringstream ss;
	ss << value;
	std::wstring str;
	ss >> str;
	return str;
}

void GetInterval()
{
	std::wstringstream ss;
	WCHAR windowtext[128] = {};
	GetWindowText(hwndInterval, windowtext, 128);
	ss << windowtext;
	ss >> state.intervalValue;
}

struct TimeFormated
{
	INT tenths;
	INT minutes;
	INT hours;
	INT days;
	TimeFormated(INT64 GameMinutesTenths)
	{
		if (state.b_TimeFormatDays)
		{
			tenths = (GameMinutesTenths % 10);
			minutes = (GameMinutesTenths % 600) / 10;
			hours = (INT)((GameMinutesTenths / 600) % 24);
			days = (INT)(GameMinutesTenths / 14400);
		}
		else
		{
			tenths = (GameMinutesTenths % 10);
			minutes = (GameMinutesTenths % 600) / 10;
			hours = (INT)(GameMinutesTenths / 600);
			days = 0;
		}
	}
};

std::wstring GetTimeString(TimeFormated timeformatted)
{
	std::wstring wsdays = L"";
	if (timeformatted.days > 0)
	{
		wsdays = std::to_wstring(timeformatted.days) + ((timeformatted.days == 1) ? L"day " : L"days ");
	}
	std::wstring wshours = L"";
	if (timeformatted.hours > 0)
	{
		wshours = std::to_wstring(timeformatted.hours) + ((timeformatted.hours == 1) ? L"h " : L"h ");
	}
	std::wstring wsminutues = std::to_wstring(timeformatted.minutes);
	return wsdays + L" " + wshours + L" " + wsminutues + L"." + std::to_wstring(timeformatted.tenths) + L"m";
}

void SetTimeformat(HWND wnd)
{
	if (state.b_TimeFormatDays)

	{
		ModifyMenuW(GetMenu(wnd), ID_TIMEFORMATDAYS, MF_BYCOMMAND | MF_CHECKED, ID_TIMEFORMATDAYS, L"&Days and Hours");
		ModifyMenuW(GetMenu(wnd), ID_TIMEFORMATHOURS, MF_BYCOMMAND | MF_UNCHECKED, ID_TIMEFORMATHOURS, L"&Hours");
	}
	else
	{
		ModifyMenuW(GetMenu(wnd), ID_TIMEFORMATDAYS, MF_BYCOMMAND | MF_UNCHECKED, ID_TIMEFORMATDAYS, L"&Days and Hours");
		ModifyMenuW(GetMenu(wnd), ID_TIMEFORMATHOURS, MF_BYCOMMAND | MF_CHECKED, ID_TIMEFORMATHOURS, L"&Hours");
	}
	RecalculateSplitStrings();
	Timerproc(0, 0, 0, 0);
}

INT64 GetTenths(std::chrono::milliseconds ms)
{
	return ms.count() / 500;
}

void Timerproc(HWND unnamedParam1, UINT unnamedParam2, UINT_PTR unnamedParam3, DWORD unnamedParam4)
{
	auto timepointnow = std::chrono::system_clock::now();
	state.timeduration = (timepointnow - state.timebegin);

	if (!state.b_StartedState)
	{
		state.timeduration = {};
	}
	state.timeduration += state.Previoustimeduration;

	GameMinutesTenths = GetTenths(std::chrono::duration_cast<std::chrono::milliseconds>(state.timeduration)) + state.AddedTime;
	if (GameMinutesTenths < 0)
	{
		state.AddedTime = -GetTenths(std::chrono::duration_cast<std::chrono::milliseconds>(state.timeduration));
		GameMinutesTenths = 0;
	}

	TimeSinceLastSplit = GameMinutesTenths - state.SplitsTenths[0];
	if (TimeSinceLastSplit < 0)
		TimeSinceLastSplit = 0;
	INT64 TimeSinceLastSplitMinutes = TimeSinceLastSplit / 10;

	INT64 GameMinutes = GameMinutesTenths / 10;
	TimeFormated timeformatted(GameMinutesTenths);
	TimeFormated timeformattedLastSplit(TimeSinceLastSplit);

	if (state.intervalValue > 0 && TimeSinceLastSplitMinutes != 0 && TimeSinceLastSplitMinutes % state.intervalValue == 0 && TimeSinceLastSplitMinutes != LastGameMinutes && TimeSinceLastSplit % 10 == 0)
	{
		if (state.b_StartedState)
			PlaySound(MAKEINTRESOURCE(IDSOUNDALARM), GetModuleHandle(NULL), SND_RESOURCE | SND_ASYNC);
		LastGameMinutes = TimeSinceLastSplitMinutes;
	}
	TimeSinceLastSplitString = GetTimeString(timeformattedLastSplit);
	TimerString = GetTimeString(timeformatted);
	InvalidateRect(hMainWindow, &redrawrect, true);
}

void StartTimer()
{
	SetDlgItemText(hMainWindow, IDM_STARTBUTTON, L"'P' Stop");
	//Create Timer that calls Timerproc each tenth of a second
	SetTimer(hMainWindow,            // handle to main window 
		IDT_TIMER1,					 // timer identifier 
		100,						 // 0.1 second interval 
		Timerproc);					 // Callback function 
}

void PauseTimer()
{
	SetDlgItemText(hMainWindow, IDM_STARTBUTTON, L"'P' Start");
	state.Previoustimeduration = state.timeduration;
	KillTimer(hMainWindow, IDT_TIMER1);
}

void RecalculateSplitStrings()
{
	SplitStrings[3] = GetTimeString(TimeFormated(state.SplitsTenths[3]));
	SplitStrings[2] = GetTimeString(TimeFormated(state.SplitsTenths[2]));
	SplitStrings[1] = GetTimeString(TimeFormated(state.SplitsTenths[1]));
	SplitStrings[0] = GetTimeString(TimeFormated(state.SplitsTenths[0]));
}

void Split()
{
	if (state.SplitsTenths[0] != GameMinutesTenths)
	{
		LastGameMinutes = 0;
		state.SplitsTenths[3] = state.SplitsTenths[2];
		state.SplitsTenths[2] = state.SplitsTenths[1];
		state.SplitsTenths[1] = state.SplitsTenths[0];
		state.SplitsTenths[0] = GameMinutesTenths;
		SplitStrings[3] = SplitStrings[2];
		SplitStrings[2] = SplitStrings[1];
		SplitStrings[1] = SplitStrings[0];
		SplitStrings[0] = TimerString;
		Timerproc(0, 0, 0, 0);
		if (state.b_Sound)
			PlaySound(MAKEINTRESOURCE(IDR_SOUNDCLICK2), GetModuleHandle(NULL), SND_RESOURCE | SND_ASYNC);
	}
}

void resetSplits()
{
	state.SplitsTenths[3] = {};
	state.SplitsTenths[2] = {};
	state.SplitsTenths[1] = {};
	state.SplitsTenths[0] = {};
	for (int i = 0; i < 4; i++)
	{
		SplitStrings[i] = GetTimeString(GetTenths({}));
	}
	if (state.b_Sound)
		PlaySound(MAKEINTRESOURCE(IDR_SOUNDCLICK3), GetModuleHandle(NULL), SND_RESOURCE | SND_ASYNC);
}

void ToggleSound(HWND wnd)
{
	if (state.b_Sound)
	{
		ModifyMenuW(GetMenu(wnd), IDM_SOUNDTOGGLE, MF_BYCOMMAND | MF_CHECKED, IDM_SOUNDTOGGLE, L"&Enabled");
	}
	else
	{
		ModifyMenuW(GetMenu(wnd), IDM_SOUNDTOGGLE, MF_BYCOMMAND | MF_UNCHECKED, IDM_SOUNDTOGGLE, L"&Enabled");
	}
}

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	SetFont();
	hInst = hInstance; // Store instance handle in our global variable

	hMainWindow = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW ^ WS_THICKFRAME ^ WS_MAXIMIZEBOX,
		CW_USEDEFAULT, 0, g_window_width + 19, g_window_height, nullptr, nullptr, hInstance, nullptr);

	if (!hMainWindow)
	{
		return FALSE;
	}

	ShowWindow(hMainWindow, nCmdShow);
	UpdateWindow(hMainWindow);

	redrawrect.left = 0;
	redrawrect.right = g_window_width;
	redrawrect.top = 0;
	redrawrect.bottom = 100;

	// Create Buttons

	hwndStartButton = CreateWindowW(
		L"BUTTON",  // Predefined class; Unicode assumed 
		L"'P' Start",      // Button text 
		WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,  // Styles 
		x1,         // x position 
		100,         // y position 
		buttonWidth,        // Button width
		buttonheight,        // Button height
		hMainWindow,     // Parent window
		(HMENU)IDM_STARTBUTTON,       // menu ID.
		(HINSTANCE)GetWindowLongPtr(hMainWindow, GWLP_HINSTANCE),
		NULL);      // Pointer not needed.

	hwndResetButton = CreateWindowW(
		L"BUTTON",  // Predefined class; Unicode assumed 
		L"'F8' Reset",      // Button text 
		WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,  // Styles 
		x2,         // x position 
		100,         // y position 
		buttonWidth,        // Button width
		buttonheight,        // Button height
		hMainWindow,     // Parent window
		(HMENU)IDM_RESETBUTTON,       // menu ID.
		(HINSTANCE)GetWindowLongPtr(hMainWindow, GWLP_HINSTANCE),
		NULL);      // Pointer not needed.

	hwndSplitButton = CreateWindowW(
		L"BUTTON",  // Predefined class; Unicode assumed 
		L"'Space' Split",      // Button text 
		WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,  // Styles 
		x3,         // x position 
		100,         // y position 
		buttonWidth,        // Button width
		buttonheight,        // Button height
		hMainWindow,     // Parent window
		(HMENU)IDM_SNAPSHOTBUTTON,       // menu ID.
		(HINSTANCE)GetWindowLongPtr(hMainWindow, GWLP_HINSTANCE),
		NULL);      // Pointer not needed.

	hwndInterval = CreateWindowEx(WS_EX_CLIENTEDGE, L"Edit", std::to_wstring(state.intervalValue).c_str(),
		WS_CHILD | WS_VISIBLE, x1, 170, 35,
		20, hMainWindow, (HMENU)IDC_INTERVALBOX, NULL, NULL);

	hWndLabel1 = CreateWindowEx(0, L"static", GetAlarmLabel().c_str(),
		WS_CHILD | WS_VISIBLE, x1, 200, 150,
		20, hMainWindow, (HMENU)IDC_STATICBOX, NULL, NULL);


	//Add minutes
	hWndAddminutes = CreateWindowEx(WS_EX_CLIENTEDGE, L"Edit", StripZeroes(state.addminutes).c_str(),
		WS_CHILD | WS_VISIBLE, x2, 170, 100,
		20, hMainWindow, (HMENU)IDC_MINUTEBOX, NULL, NULL);

	hwndSplitButton = CreateWindowW(
		L"BUTTON",  // Predefined class; Unicode assumed 
		L"Add Minutes",      // Button text 
		WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,  // Styles 
		x2,         // x position 
		200,         // y position 
		buttonWidth,        // Button width
		buttonheight,        // Button height
		hMainWindow,     // Parent window
		(HMENU)IDM_ADDMINUTEBUTTON,       // menu ID.
		(HINSTANCE)GetWindowLongPtr(hMainWindow, GWLP_HINSTANCE),
		NULL);      // Pointer not needed.

	//Add hours
	hWndAddHours = CreateWindowEx(WS_EX_CLIENTEDGE, L"Edit", StripZeroes(state.addhours).c_str(),
		WS_CHILD | WS_VISIBLE, x3, 170, 100,
		20, hMainWindow, (HMENU)IDC_HOURBOX, NULL, NULL);

	hwndSplitButton = CreateWindowW(
		L"BUTTON",  // Predefined class; Unicode assumed 
		L"Add Hours",      // Button text 
		WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,  // Styles 
		x3,         // x position 
		200,         // y position 
		buttonWidth,        // Button width
		buttonheight,        // Button height
		hMainWindow,     // Parent window
		(HMENU)IDM_ADDHOURBUTTON,       // menu ID.
		(HINSTANCE)GetWindowLongPtr(hMainWindow, GWLP_HINSTANCE),
		NULL);      // Pointer not needed.

	if (state.b_StartedState)
	{
		StartTimer();
	}
	else
	{
		state.timebegin = std::chrono::system_clock::now();
	}
	Timerproc(0, 0, 0, 0);

	for (int i = 0; i < 4; i++)
	{
		SplitStrings[i] = GetTimeString(state.SplitsTenths[i]);
	}
	ToggleSound(hMainWindow);
	SetTimeformat(hMainWindow);
	return TRUE;
}

void ToggleStopWatch()
{
	state.b_StartedState = !state.b_StartedState;
	if (state.b_StartedState)
	{
		state.timebegin = std::chrono::system_clock::now();
		StartTimer();
		LastGameMinutes = 0;
		if (state.b_Sound)
			PlaySound(MAKEINTRESOURCE(IDR_SOUNDCLICK0), GetModuleHandle(NULL), SND_RESOURCE | SND_ASYNC);
	}
	else
	{
		PauseTimer();
		if (state.b_Sound)
			PlaySound(MAKEINTRESOURCE(IDR_SOUNDCLICK1), GetModuleHandle(NULL), SND_RESOURCE | SND_ASYNC);
	}
}

void ResetStopWatch()
{
	LastGameMinutes = 0;
	resetSplits();
	if (state.b_StartedState)
	{
		state.b_StartedState = false;
		state.timebegin = std::chrono::system_clock::now();
		PauseTimer();
	}
	GameMinutesTenths = 0;
	state.AddedTime = {};
	state.timebegin = std::chrono::system_clock::now();
	state.Previoustimeduration = {};
	TimeSinceLastSplitString = GetTimeString(TimeFormated({}));
	TimerString = GetTimeString(TimeFormated({}));
	InvalidateRect(hMainWindow, &redrawrect, true);
	if (state.b_Sound)
		PlaySound(MAKEINTRESOURCE(IDR_SOUNDCLICK4), GetModuleHandle(NULL), SND_RESOURCE | SND_ASYNC);
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_COMMAND:
	{
		int wmId = LOWORD(wParam);
		switch (wmId)
		{
		case IDM_ABOUT:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			break;
		case IDM_SOUNDTOGGLE:
			state.b_Sound = !state.b_Sound;
			if (state.b_Sound)
			{
				PlaySound(MAKEINTRESOURCE(IDR_SOUNDCLICK1), GetModuleHandle(NULL), SND_RESOURCE | SND_ASYNC);
			}
			ToggleSound(hMainWindow);
			break;
		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;
		case IDM_STARTBUTTON:
			SetFocus(hMainWindow);
			ToggleStopWatch();
			break;
		case IDM_RESETBUTTON:
			SetFocus(hMainWindow);
			ResetStopWatch();
			break;
		case IDM_SNAPSHOTBUTTON:
		{
			SetFocus(hMainWindow);
			Split();
		}
		break;
		case IDM_SPLITRESET:
		{
			resetSplits();
			InvalidateRect(hMainWindow, &redrawrect, true);
		}
		break;
		case IDC_INTERVALBOX:
			if (HIWORD(wParam) == EN_CHANGE)
			{
				GetInterval();
				SetDlgItemText(hWnd, IDC_STATICBOX, GetAlarmLabel().c_str());
			}
			break;
		case IDM_ADDMINUTEBUTTON:
		{
			SetFocus(hMainWindow);
			state.AddedTime += INT64(state.addminutes * 10.0);
			Timerproc(0, 0, 0, 0);
			if (state.b_Sound)
				PlaySound(MAKEINTRESOURCE(IDR_SOUNDCLICK2), GetModuleHandle(NULL), SND_RESOURCE | SND_ASYNC);
		}
		break;
		case IDM_ADDHOURBUTTON:
		{
			SetFocus(hMainWindow);
			state.AddedTime += INT64(state.addhours * 600.0);
			Timerproc(0, 0, 0, 0);
			if (state.b_Sound)
				PlaySound(MAKEINTRESOURCE(IDR_SOUNDCLICK3), GetModuleHandle(NULL), SND_RESOURCE | SND_ASYNC);
		}
		break;
		case IDC_MINUTEBOX:
			if (HIWORD(wParam) == EN_CHANGE)
			{
				std::wstringstream ss;
				WCHAR windowtext[64] = {};
				GetWindowText(hWndAddminutes, windowtext, 64);
				ss << windowtext;
				ss >> state.addminutes;
			}
			break;
		case IDC_HOURBOX:
			if (HIWORD(wParam) == EN_CHANGE)
			{
				std::wstringstream ss;
				WCHAR windowtext[64] = {};
				GetWindowText(hWndAddHours, windowtext, 64);
				ss << windowtext;
				ss >> state.addhours;
			}
			break;
		case ID_TIMEFORMATDAYS:
			state.b_TimeFormatDays = TRUE;
			SetTimeformat(hMainWindow);
			break;
		case ID_TIMEFORMATHOURS:
			state.b_TimeFormatDays = FALSE;
			SetTimeformat(hMainWindow);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
	}
	break;
	case WM_LBUTTONDOWN:
		SetFocus(hMainWindow);
		break;
	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hWnd, &ps);
		SelectObject(hdc, splitfont);
		TextOutW(hdc, 35, 18, TimerString.c_str(), (int)TimerString.size());
		SelectObject(hdc, myfont1);
		TextOutW(hdc, 35, 54, TimeSinceLastSplitString.c_str(), (int)TimeSinceLastSplitString.size());
		SelectObject(hdc, splitfont);
		for (int i = 0; i < 4; i++)
		{
			TextOutW(hdc, x2 + buttonWidth, 10 + i * 22, SplitStrings[i].c_str(), (int)SplitStrings[i].size());
		}
		EndPaint(hWnd, &ps);
	}
	break;
	case WM_DESTROY:
		SavetoFile(state);
		DeleteObject(myfont1);
		DeleteObject(splitfont);
		PostQuitMessage(0);
		break;
	case WM_KEYDOWN:
	{
		switch (wParam)
		{
		case 0x50:
		{
			ToggleStopWatch();
			InvalidateRect(hMainWindow, &redrawrect, true);
		}
		break;
		case VK_F8:
		{
			ResetStopWatch();
			InvalidateRect(hMainWindow, &redrawrect, true);
		}
		break;
		case VK_SPACE:
		{
			Split();
			InvalidateRect(hMainWindow, &redrawrect, true);
		}
		break;
		case VK_F5:
		{
			resetSplits();
			InvalidateRect(hMainWindow, &redrawrect, true);
		}
		break;
		}
	}
	break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}


SavedState LoadFromFile()
{
	SavedState ss = {};
	std::ifstream is;
	is.open("Timer.data", std::ios::binary);
	if (is.is_open())
	{
		is.read(reinterpret_cast<char*>(&ss), sizeof(ss));
		is.close();
	}
	return ss;
}

void SavetoFile(SavedState ss)
{
	std::ofstream file;
	file.open("Timer.data", std::ios::binary);
	if (file.is_open())
	{
		file.write(reinterpret_cast<char*>(&state), sizeof(state));
		file.close();
	}
}