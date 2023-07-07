// The Long Dark Game Time StopWatch.cpp : Defines the entry point for the application.
//

#include "framework.h"
#include "The Long Dark Game Time StopWatch.h"
#include <string>
#include <sstream>
#include <chrono>

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

int g_window_width = 400;
int g_window_height = 250;
int buttonWidth = 120;
int buttonheight = 50;

HWND hMainWindow = nullptr;
HWND hwndStartButton = nullptr;
HWND hwndResetButton = nullptr;

BOOL b_state = false;

std::wstring TimerString = L"";

HFONT myfont = nullptr;

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

std::chrono::time_point<std::chrono::system_clock> timebegin = std::chrono::system_clock::now();
std::chrono::duration<double> timeduration = {};
std::chrono::duration<double> Previoustimeduration = {};

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

void SetFont()
{
	LOGFONTW font = { 0 };

	font.lfHeight = 30;
	font.lfWidth = 12;

	myfont = CreateFontIndirectW(&font);
}


void Timerproc(HWND unnamedParam1, UINT unnamedParam2, UINT_PTR unnamedParam3, DWORD unnamedParam4)
{
	if (b_state)
	{
		auto timepointnow = std::chrono::system_clock::now();
		timeduration = (timepointnow - timebegin) + Previoustimeduration;

		auto x = std::chrono::duration_cast<std::chrono::milliseconds>(timeduration);

		UINT64 GameMinutesTenths = x.count() / 500;
		UINT tenths = (GameMinutesTenths % 10);
		UINT minutes = (GameMinutesTenths % 600) / 10;
		UINT hours = (UINT)(GameMinutesTenths / 600);

		std::wstring wshours = L"";
		if (hours > 0)
		{
			wshours = std::to_wstring(hours) + ((hours == 1) ? L" hour " : L" hours ");
		}
		std::wstring wsminutues = std::to_wstring(minutes);

		TimerString = wshours + L" " + wsminutues + L"." + std::to_wstring(tenths) + L" mins";
		TimerString = TimerString + L"                                                 ";

		InvalidateRect(hMainWindow, nullptr, false);
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
	int x1 = int(g_window_width / 4.0f - buttonWidth / 2.0f);
	int x2 = int(g_window_width * (3.0f / 4.0f) - buttonWidth / 2.0f);
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
			b_state = !b_state;
			if (b_state)
			{
				timebegin = std::chrono::system_clock::now();
				SetDlgItemText(hWnd, IDM_STARTBUTTON, L"Stop");

				//Create Timer that calls Timerproc each tenth of a second
				SetTimer(hMainWindow,            // handle to main window 
					IDT_TIMER1,					 // timer identifier 
					100,						 // 0.1 second interval 
					Timerproc);					 // no timer callback 
			}
			else
			{
				SetDlgItemText(hWnd, IDM_STARTBUTTON, L"Start");
				Previoustimeduration = timeduration;
				//Stops Timer
				KillTimer(hMainWindow, IDT_TIMER1);
			}
			InvalidateRect(hWnd, nullptr, false);
			break;
		case IDM_RESETBUTTON:
			KillTimer(hMainWindow, IDT_TIMER1);
			Previoustimeduration = {};
			TimerString = L"                                                 ";
			b_state = false;
			SetDlgItemText(hWnd, IDM_STARTBUTTON, L"Start");
			InvalidateRect(hMainWindow, nullptr, false);
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
		SelectObject(hdc, myfont);
		TextOutW(hdc, 35, 36, TimerString.c_str(), (int)TimerString.size());

		EndPaint(hWnd, &ps);
	}
	break;
	case WM_DESTROY:
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
