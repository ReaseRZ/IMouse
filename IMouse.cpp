#include <windows.h>
#include <CommCtrl.h>
#include <stdio.h>

#define AUTOSCRL 0
#define AUTOCLK 1
#define SPDUP 2
#define SPDDWN 3
#define CLKUP 4
#define CLKDWN 5

#define WIDTH 400
#define HEIGHT 400

BOOL running = FALSE;
BOOL running_sec = FALSE;
BOOL iScrollActive = FALSE;
BOOL IClickActive = FALSE;
DWORD tid = 0;
DWORD tid2 = 1;
DWORD spd = 0;
DWORD click = 1;
HWND  hwnd;
HANDLE thread;

struct {
	int iStyle;
	TCHAR *text;
}button[] =
{
	BS_PUSHBUTTON,TEXT("AUTOSCROLL"),
	BS_PUSHBUTTON,TEXT("+"),
	BS_PUSHBUTTON,TEXT("-"),
	BS_PUSHBUTTON,TEXT("AUTOCLICK")
};

struct {
	TCHAR *text;
}textList[]=
{
	TEXT("AUTOSCROLL IS ON"),
	TEXT("AUTOSCROLL IS OFF"),
	TEXT("AUTOCLICK activated, click F5 to turn OFF"),
	TEXT("AUTOCLICK is not active")
};


void mouse_wheel(void) {
	mouse_event(MOUSEEVENTF_WHEEL, 0, 0, spd, 0);
}

void mouse_button(void) {
	mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0);
	mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
}

DWORD WINAPI MyThreadMouseWheelFunction(LPVOID lpParam) {
	int* r = (int*)lpParam;
	while (*r) {
		mouse_wheel();
		Sleep(100);
	}
	return 0;
}

DWORD WINAPI MyThreadMouseButtonFunction(LPVOID lpParam) {
	int* r = (int*)lpParam;
	while (*r) {
		mouse_button();
		Sleep(1000/click);
	}
	return 0;
}

LRESULT CALLBACK LowLevelMouseProc(int nCode, WPARAM wParam, LPARAM lParam) {
	bool fEatKeyStroke = false;
	static RECT rc;
	if (nCode == HC_ACTION) {
		switch (wParam) {
		case WM_KEYDOWN:
		case WM_SYSKEYDOWN:
		case WM_KEYUP:
		case WM_SYSKEYUP:
			GetClientRect(hwnd, &rc);
			PKBDLLHOOKSTRUCT p = (PKBDLLHOOKSTRUCT)lParam;
			if (fEatKeyStroke = (p->vkCode == VK_F5)) {
				if (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN) {
					if (IClickActive) {
						running_sec = FALSE;
						IClickActive = FALSE;
						CloseHandle(thread);
					}
				}
			}
			InvalidateRect(hwnd, &rc, TRUE);
		}
	}
	return (fEatKeyStroke ? 1 : CallNextHookEx(NULL, nCode, wParam, lParam));
}

LRESULT CALLBACK WndProc (HWND, UINT, WPARAM, LPARAM) ;

HINSTANCE hInst ;

int WINAPI WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance,
                    PSTR szCmdLine, int iCmdShow)
{
     static TCHAR szAppName[] = TEXT ("OwnDraw") ;
     MSG          msg ;
     WNDCLASS     wndclass ;
     
     hInst = hInstance ;
     
     wndclass.style         = CS_HREDRAW | CS_VREDRAW ;
     wndclass.lpfnWndProc   = WndProc ;
     wndclass.cbClsExtra    = 0 ;
     wndclass.cbWndExtra    = 0 ;
     wndclass.hInstance     = hInstance ;
     wndclass.hIcon         = LoadIcon (NULL, IDI_APPLICATION) ;
     wndclass.hCursor       = LoadCursor (NULL, IDC_ARROW) ;
     wndclass.hbrBackground = (HBRUSH) GetStockObject (WHITE_BRUSH) ;
     wndclass.lpszMenuName  = szAppName ;
     wndclass.lpszClassName = szAppName ;
     
     if (!RegisterClass (&wndclass))
     {
          MessageBox (NULL, TEXT ("Can't open program"),
                      szAppName, MB_ICONERROR) ;
          return 0 ;
     }
     
     hwnd = CreateWindow (szAppName, TEXT ("IMouse"),
                          WS_CAPTION,
                          CW_USEDEFAULT, CW_USEDEFAULT,
                          WIDTH, HEIGHT,
                          NULL, NULL, hInstance, NULL) ;
     
     ShowWindow (hwnd, iCmdShow) ;
     UpdateWindow (hwnd) ; 

	 HHOOK hhkLowLevelKybd = SetWindowsHookEx(WH_KEYBOARD_LL, LowLevelMouseProc, 0, 0);
     
     while (GetMessage (&msg, NULL, 0, 0))
     {
          TranslateMessage (&msg) ;
          DispatchMessage (&msg) ;
     }
     return msg.wParam ;
}

LRESULT CALLBACK WndProc (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
     static HWND      AutoScrollBtn, AutoClickBtn, SpeedUpButton,SpeedDownButton,ClickRateUp,ClickRateDown;
	 HDC			  hdc;
	 PAINTSTRUCT      lp;
	 static RECT	  rc;
	 TCHAR			  szBuffer[25];
	 TCHAR			  szCBuffer[25];
	 int			  length;
	 int			  cLength;
     
     switch (message)
     {
     case WM_CREATE :
		 GetClientRect(hwnd, &rc);
		 SpeedUpButton = CreateWindow(TEXT("button"), button[1].text, WS_CHILD | WS_VISIBLE | button[1].iStyle, WIDTH / 2 - 150, 125, 50, 50, hwnd, (HMENU)SPDUP, ((LPCREATESTRUCT)lParam)->hInstance, NULL);
		 AutoScrollBtn = CreateWindow(TEXT("button"), button[0].text, WS_CHILD | WS_VISIBLE | button[0].iStyle, WIDTH/2-75, 125, 150, 50, hwnd, (HMENU)AUTOSCRL, ((LPCREATESTRUCT)lParam)->hInstance, NULL);
		 SpeedDownButton = CreateWindow(TEXT("button"), button[2].text, WS_CHILD | WS_VISIBLE | button[2].iStyle, WIDTH / 2+100 , 125, 50, 50, hwnd, (HMENU)SPDDWN, ((LPCREATESTRUCT)lParam)->hInstance, NULL);
		 AutoClickBtn = CreateWindow(TEXT("button"), button[3].text, WS_CHILD | WS_VISIBLE | button[3].iStyle, WIDTH / 2 - 75, 275, 150, 50, hwnd, (HMENU)AUTOCLK, ((LPCREATESTRUCT)lParam)->hInstance, NULL);
		 ClickRateUp = CreateWindow(TEXT("button"), button[1].text, WS_CHILD | WS_VISIBLE | button[1].iStyle, WIDTH / 2 - 150, 275, 50, 50, hwnd, (HMENU)CLKUP, ((LPCREATESTRUCT)lParam)->hInstance, NULL);
		 ClickRateDown = CreateWindow(TEXT("button"), button[2].text, WS_CHILD | WS_VISIBLE | button[2].iStyle, WIDTH / 2 + 100, 275, 50, 50, hwnd, (HMENU)CLKDWN, ((LPCREATESTRUCT)lParam)->hInstance, NULL);
          return 0 ;
	 case WM_COMMAND:
		 switch (wParam) {
		 case AUTOSCRL:
			 if (running == FALSE) {
				 running = TRUE;
				 iScrollActive = TRUE;
				 thread = CreateThread(NULL, 0, MyThreadMouseWheelFunction, &running, 0, &tid);
			 }
			 else {
				 running = FALSE;
				 iScrollActive = FALSE;
				 CloseHandle(thread);
			 }
			 break;
		 case AUTOCLK:
			 if (running_sec == FALSE) {
				 IClickActive = TRUE;
				 running_sec = TRUE;
				 Sleep(1500);
				 thread = CreateThread(NULL, 0, MyThreadMouseButtonFunction, &running_sec, 0, &tid);
			 }
			 else {
				 running_sec = FALSE;
				 IClickActive = FALSE;
				 CloseHandle(thread);
			 }
			 break;
		 case SPDDWN:
			 spd-=15;
			 break;
		 case SPDUP:
			 spd+=15;
			 break;
		 case CLKDWN:
			 if (click > 1) {
				 click--;
			 }
			 break;
		 case CLKUP:
			 click++;
			 break;
		 }

		 InvalidateRect(hwnd, &rc, TRUE);
		 return 0;
	 case WM_RBUTTONDOWN:
		 if (running_sec == TRUE) {
			 running_sec = FALSE;
			 IClickActive = FALSE;
			 CloseHandle(thread);
		 }
		 InvalidateRect(hwnd, &rc, TRUE);
		 return 0;
	 case WM_PAINT:
		 hdc = BeginPaint(hwnd, &lp);
		 SetTextAlign(hdc, TA_CENTER);
		 length = wsprintf(szBuffer, TEXT("SCROLL SPEED : [%i]"), spd);
		 cLength = wsprintf(szCBuffer, TEXT("CLICK PER SECOND : [%i]"), click);

		 if (iScrollActive) {
			 TextOut(hdc, WIDTH/2, 50, textList[0].text, lstrlen(textList[0].text));
		 }
		 else {
			 TextOut(hdc, WIDTH / 2, 50, textList[1].text, lstrlen(textList[1].text));
		 }
		 TextOut(hdc, WIDTH / 2, 90, szBuffer, length);
		 if (IClickActive) {
			 TextOut(hdc, WIDTH/2, 200, textList[2].text, lstrlen(textList[2].text));
		 }
		 else {
			 TextOut(hdc, WIDTH / 2, 200, textList[3].text, lstrlen(textList[3].text));
		 }
		 TextOut(hdc, WIDTH / 2, 240, szCBuffer, cLength);
		 EndPaint(hwnd, &lp);
          return 0 ;
          
     case WM_DESTROY :
          PostQuitMessage (0) ;
          return 0 ;
     }
     return DefWindowProc (hwnd, message, wParam, lParam) ;
}