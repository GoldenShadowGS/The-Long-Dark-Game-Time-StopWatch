// The Long Dark Game Time StopWatch.cpp : Defines the entry point for the application.
//

#include "framework.h"
#include "StopWatch.h"
#include <string>
#include <sstream>
#include <chrono>
#include <fstream>

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

struct SavedState
{
	std::chrono::time_point<std::chrono::system_clock> timebegin;
	std::chrono::duration<double> timeduration;;
	std::chrono::duration<double> Previoustimeduration;
	std::chrono::duration<double> Splits[4];
	UINT intervalValue;
	BOOL b_StartedState;
	BOOL b_Sound;
};

SavedState LoadFromFile();
void SavetoFile(SavedState ss);

int g_window_width = 500;
int g_window_height = 300;
int buttonWidth = 120;
int buttonheight = 50;

HWND hMainWindow = nullptr;
HWND hwndStartButton = nullptr;
HWND hwndResetButton = nullptr;
HWND hwndSplitButton = nullptr;
HWND hwndInterval = nullptr;
HWND hWndLabel1 = nullptr;

std::wstring TimerString = L"";
std::wstring SplitStrings[4];

HFONT myfont1 = nullptr;
HFONT splitfonts[4] = {};

std::chrono::milliseconds g_ms = {};

SavedState state = LoadFromFile();

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	// TODO: Place code here.

	// Initialize global strings
	LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadStringW(hInstance, IDC_THELONGDARKGAMETIMESTOPWATCH, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// Perform application initialization:
	if (!InitInstance(hInstance, nCmdShow))
	{
		return FALSE;
	}

	HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_THELONGDARKGAMETIMESTOPWATCH));

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


//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEXW wcex = {};

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_THELONGDARKGAMETIMESTOPWATCH));
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_THELONGDARKGAMETIMESTOPWATCH);
	wcex.lpszClassName = szWindowClass;
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassExW(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//

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
	float scale[4] = { 0.75f, 0.70f, 0.60f, 0.55f };
	for (int i = 0; i < 4; i++)
	{
		font.lfHeight = int(30 * scale[i]);
		font.lfWidth = int(12 * scale[i]);
		splitfonts[i] = CreateFontIndirectW(&font);
	}
}

void GetInterval()
{
	std::wstringstream ss;
	WCHAR windowtext[128] = {};
	GetWindowText(hwndInterval, windowtext, 128);
	ss << windowtext;
	ss >> state.intervalValue;
}

std::wstring GetTimeString(std::chrono::milliseconds ms)
{
	UINT64 GameMinutesTenths = ms.count() / 500;
	UINT64 GameMinutes = GameMinutesTenths / 10;
	static UINT64 LastGameMinutes = 0;
	UINT tenths = (GameMinutesTenths % 10);
	UINT minutes = (GameMinutesTenths % 600) / 10;
	UINT hours = (UINT)(GameMinutesTenths / 600);

	std::wstring wshours = L"";
	if (hours > 0)
	{
		wshours = std::to_wstring(hours) + ((hours == 1) ? L" hour " : L" hours ");
	}
	std::wstring wsminutues = std::to_wstring(minutes);
	return wshours + L" " + wsminutues + L"." + std::to_wstring(tenths) + L" m                                                  ";
}

void Timerproc(HWND unnamedParam1, UINT unnamedParam2, UINT_PTR unnamedParam3, DWORD unnamedParam4)
{
	auto timepointnow = std::chrono::system_clock::now();
	state.timeduration = (timepointnow - state.timebegin) + state.Previoustimeduration;

	g_ms = std::chrono::duration_cast<std::chrono::milliseconds>(state.timeduration);

	UINT64 GameMinutesTenths = g_ms.count() / 500;
	UINT64 GameMinutes = GameMinutesTenths / 10;
	static UINT64 LastGameMinutes = 0;
	UINT tenths = (GameMinutesTenths % 10);
	UINT minutes = (GameMinutesTenths % 600) / 10;
	UINT hours = (UINT)(GameMinutesTenths / 600);
	static BOOL FirstStart = true;
	if (FirstStart)
	{
		LastGameMinutes = GameMinutes;
		FirstStart = false;
	}

	if (state.intervalValue > 0 && GameMinutes != 0 && GameMinutes % state.intervalValue == 0 && (GameMinutes != LastGameMinutes))
	{
		PlaySound(MAKEINTRESOURCE(IDSOUNDALARM), GetModuleHandle(NULL), SND_RESOURCE | SND_ASYNC);
		LastGameMinutes = GameMinutes;
	}

	std::wstring wshours = L"";
	if (hours > 0)
	{
		wshours = std::to_wstring(hours) + ((hours == 1) ? L" hour " : L" hours ");
	}
	std::wstring wsminutues = std::to_wstring(minutes);

	TimerString = wshours + L" " + wsminutues + L"." + std::to_wstring(tenths) + L" m                                                  ";

	InvalidateRect(hMainWindow, nullptr, false);
}

void StartTimer()
{
	SetDlgItemText(hMainWindow, IDM_STARTBUTTON, L"Stop");

	//Create Timer that calls Timerproc each tenth of a second
	SetTimer(hMainWindow,            // handle to main window 
		IDT_TIMER1,					 // timer identifier 
		100,						 // 0.1 second interval 
		Timerproc);					 // no timer callback 
}

void PauseTimer()
{
	SetDlgItemText(hMainWindow, IDM_STARTBUTTON, L"Start");
	state.Previoustimeduration = state.timeduration;
	//Stops Timer
	KillTimer(hMainWindow, IDT_TIMER1);
}

void Split()
{
	state.Splits[3] = state.Splits[2];
	state.Splits[2] = state.Splits[1];
	state.Splits[1] = state.Splits[0];
	state.Splits[0] = g_ms;
	SplitStrings[3] = SplitStrings[2];
	SplitStrings[2] = SplitStrings[1];
	SplitStrings[1] = SplitStrings[0];
	SplitStrings[0] = TimerString;
}

void resetSplits()
{
	state.Splits[3] = {};
	state.Splits[2] = {};
	state.Splits[1] = {};
	state.Splits[0] = {};
	for (int i = 0; i < 4; i++)
	{
		SplitStrings[i] = GetTimeString(std::chrono::duration_cast<std::chrono::milliseconds>(state.Splits[i]));
	}
}

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	SetFont();
	hInst = hInstance; // Store instance handle in our global variable

	hMainWindow = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW ^ WS_THICKFRAME,
		CW_USEDEFAULT, 0, g_window_width, g_window_height, nullptr, nullptr, hInstance, nullptr);

	if (!hMainWindow)
	{
		return FALSE;
	}

	ShowWindow(hMainWindow, nCmdShow);
	UpdateWindow(hMainWindow);

	// Create Buttons
	float sixthx = g_window_width / 6.0f;
	int x1 = int(sixthx - buttonWidth / 2.0f);
	int x2 = int(sixthx * 3 - buttonWidth / 2.0f);
	int x3 = int(sixthx * 5 - buttonWidth / 2.0f);
	hwndStartButton = CreateWindowW(
		L"BUTTON",  // Predefined class; Unicode assumed 
		L"Start",      // Button text 
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
		L"Reset",      // Button text 
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
		L"Split",      // Button text 
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
		WS_CHILD | WS_VISIBLE, x1, 155, 35,
		20, hMainWindow, (HMENU)IDC_INTERVALBOX, NULL, NULL);

	hWndLabel1 = CreateWindowEx(0, L"static", GetAlarmLabel().c_str(),
		WS_CHILD | WS_VISIBLE, x1, 185, 180,
		20, hMainWindow, (HMENU)IDC_STATICBOX, NULL, NULL);

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
		SplitStrings[i] = GetTimeString(std::chrono::duration_cast<std::chrono::milliseconds>(state.Splits[i]));
	}

	return TRUE;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_COMMAND:
	{
		int wmId = LOWORD(wParam);
		// Parse the menu selections:
		switch (wmId)
		{
		case IDM_ABOUT:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			break;
		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;
		case IDM_STARTBUTTON:
			state.b_StartedState = !state.b_StartedState;
			if (state.b_StartedState)
			{
				state.timebegin = std::chrono::system_clock::now();
				StartTimer();
			}
			else
			{
				PauseTimer();
			}
			break;
		case IDM_RESETBUTTON:
			PauseTimer();
			resetSplits();
			state.timebegin = std::chrono::system_clock::now();
			state.Previoustimeduration = {};
			TimerString = L"                                                 ";
			state.b_StartedState = false;
			SetDlgItemText(hWnd, IDM_STARTBUTTON, L"Start");
			Timerproc(0, 0, 0, 0);
			break;
		case IDM_SNAPSHOTBUTTON:
		{
			if (state.b_StartedState)
			{
				Split();

			}
			else
			{
				PlaySound(MAKEINTRESOURCE(IDSOUNDERROR), GetModuleHandle(NULL), SND_RESOURCE | SND_ASYNC);
			}


			//std::wstring beepstring;
			//beepstring = L"Snap Shot";
			//SetDlgItemText(hWnd, IDC_STATICBOX, beepstring.c_str());
		}
		break;
		case IDC_INTERVALBOX:
			if (HIWORD(wParam) == EN_CHANGE)
			{
				GetInterval();
				SetDlgItemText(hWnd, IDC_STATICBOX, GetAlarmLabel().c_str());
			}
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
	}
	break;
	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hWnd, &ps);
		// TODO: Add any drawing code that uses hdc here...
		SelectObject(hdc, myfont1);
		TextOutW(hdc, 35, 36, TimerString.c_str(), (int)TimerString.size());
		for (int i = 0; i < 4; i++)
		{
			SelectObject(hdc, splitfonts[i]);
			TextOutW(hdc, g_window_width / 2 + 35 + i * 4, 76 - i * 22, SplitStrings[i].c_str(), (int)SplitStrings[i].size());
		}
		EndPaint(hWnd, &ps);
	}
	break;
	case WM_DESTROY:
		PauseTimer();
		SavetoFile(state);
		DeleteObject(myfont1);
		for (int i = 0; i < 4; i++)
		{
			DeleteObject(splitfonts[i]);
		}
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

// Message handler for about box.
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