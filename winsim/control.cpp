#include <Windows.h>

LRESULT CALLBACK windowprocessforwindow1(HWND handleforwindow1, UINT message, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK windowprocessforwindow2(HWND handleforwindow1, UINT message, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK windowprocessforwindow3(HWND handleforwindow1, UINT message, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK windowprocessforwindow4(HWND handleforwindow1, UINT message, WPARAM wParam, LPARAM lParam);

#define createwindowbuttoninwindow1 101
#define createwindowbuttoninwindow2 201
#define createwindowbuttoninwindow3 301

bool window1open, window2open, window3open, window4open = false;
bool windowclass1registeredbefore, windowclass2registeredbefore,
windowclass3registeredbefore, windowclass4registeredbefore = false;

enum windowtoopenenumt { none, window2, window3, window4 };

windowtoopenenumt windowtoopenenum = none;

void createwindow2(WNDCLASSEX& wc, HWND& hwnd, HINSTANCE hInst, int nShowCmd);
void createwindow3(WNDCLASSEX& wc, HWND& hwnd, HINSTANCE hInst, int nShowCmd);
void createwindow4(WNDCLASSEX& wc, HWND& hwnd, HINSTANCE hInst, int nShowCmd);

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR lpCmdLine, int nShowCmd)
{
   bool endprogram = false;
   WNDCLASSEX windowclassforwindow2;
   WNDCLASSEX windowclassforwindow3;
   WNDCLASSEX windowclassforwindow4;
   HWND handleforwindow2;
   HWND handleforwindow3;
   HWND handleforwindow4;

   //create window 1
   MSG msg;
   WNDCLASSEX windowclassforwindow1;
   ZeroMemory(&windowclassforwindow1, sizeof(WNDCLASSEX));
   windowclassforwindow1.cbClsExtra = NULL;
   windowclassforwindow1.cbSize = sizeof(WNDCLASSEX);
   windowclassforwindow1.cbWndExtra = NULL;
   windowclassforwindow1.hbrBackground = (HBRUSH)COLOR_WINDOW;
   windowclassforwindow1.hCursor = LoadCursor(NULL, IDC_ARROW);
   windowclassforwindow1.hIcon = NULL;
   windowclassforwindow1.hIconSm = NULL;
   windowclassforwindow1.hInstance = hInst;
   windowclassforwindow1.lpfnWndProc = (WNDPROC)windowprocessforwindow1;
   windowclassforwindow1.lpszClassName = L"window class 1";
   windowclassforwindow1.lpszMenuName = NULL;
   windowclassforwindow1.style = CS_HREDRAW | CS_VREDRAW;

   if (!RegisterClassEx(&windowclassforwindow1))
   {
      int nResult = GetLastError();
      MessageBox(NULL,
         L"Window class creation failed",
         L"Window Class Failed",
         MB_ICONERROR);
   }

   HWND handleforwindow1 = CreateWindowEx(NULL,
      windowclassforwindow1.lpszClassName,
      L"Window 1",
      WS_OVERLAPPEDWINDOW,
      200,
      150,
      640,
      480,
      NULL,
      NULL,
      hInst,
      NULL                /* No Window Creation data */
      );

   if (!handleforwindow1)
   {
      int nResult = GetLastError();

      MessageBox(NULL,
         L"Window creation failed",
         L"Window Creation Failed",
         MB_ICONERROR);
   }

   ShowWindow(handleforwindow1, nShowCmd);
   bool endloop = false;
   while (endloop == false) {
      if (GetMessage(&msg, NULL, 0, 0));
      {
         TranslateMessage(&msg);
         DispatchMessage(&msg);
      }

      if (windowtoopenenum != none) {
         switch (windowtoopenenum) {
         case window2:
            if (window2open == false) {
               createwindow2(windowclassforwindow2, handleforwindow2, hInst, nShowCmd);
            }
            break;
         case window3:
            if (window3open == false) {
               createwindow3(windowclassforwindow3, handleforwindow3, hInst, nShowCmd);
            }
            break;
         case window4:
            if (window4open == false) {
               createwindow4(windowclassforwindow4, handleforwindow4, hInst, nShowCmd);
            }
            break;
         }
         windowtoopenenum = none;
      }
      if (window1open == false && window2open == false && window3open == false && window4open == false)
         endloop = true;

   }
   MessageBox(NULL,
      L"All Windows are closed.  Program will now close.",
      L"Message",
      MB_ICONINFORMATION);

}

void createwindow2(WNDCLASSEX& wc, HWND& hwnd, HINSTANCE hInst, int nShowCmd) {
   if (windowclass2registeredbefore == false) {
      ZeroMemory(&wc, sizeof(WNDCLASSEX));
      wc.cbClsExtra = NULL;
      wc.cbSize = sizeof(WNDCLASSEX);
      wc.cbWndExtra = NULL;
      wc.hbrBackground = (HBRUSH)COLOR_WINDOW;
      wc.hCursor = LoadCursor(NULL, IDC_ARROW);
      wc.hIcon = NULL;
      wc.hIconSm = NULL;
      wc.hInstance = hInst;
      wc.lpfnWndProc = (WNDPROC)windowprocessforwindow2;
      wc.lpszClassName = L"wc2";
      wc.lpszMenuName = NULL;
      wc.style = CS_HREDRAW | CS_VREDRAW;

      if (!RegisterClassEx(&wc))
      {
         int nResult = GetLastError();
         MessageBox(NULL,
            L"Window class creation failed",
            L"Window Class Failed",
            MB_ICONERROR);
      }
      else
         windowclass2registeredbefore = true;
   }
   hwnd = CreateWindowEx(NULL,
      wc.lpszClassName,
      L"Window 2",
      WS_OVERLAPPEDWINDOW,
      200,
      170,
      640,
      480,
      NULL,
      NULL,
      hInst,
      NULL                /* No Window Creation data */
      );

   if (!hwnd)
   {
      int nResult = GetLastError();

      MessageBox(NULL,
         L"Window creation failed",
         L"Window Creation Failed",
         MB_ICONERROR);
   }

   ShowWindow(hwnd, nShowCmd);
}

void createwindow3(WNDCLASSEX& wc, HWND& hwnd, HINSTANCE hInst, int nShowCmd) {
   if (windowclass3registeredbefore == false) {
      ZeroMemory(&wc, sizeof(WNDCLASSEX));
      wc.cbClsExtra = NULL;
      wc.cbSize = sizeof(WNDCLASSEX);
      wc.cbWndExtra = NULL;
      wc.hbrBackground = (HBRUSH)COLOR_WINDOW;
      wc.hCursor = LoadCursor(NULL, IDC_ARROW);
      wc.hIcon = NULL;
      wc.hIconSm = NULL;
      wc.hInstance = hInst;
      wc.lpfnWndProc = (WNDPROC)windowprocessforwindow3;
      wc.lpszClassName = L"window class 3";
      wc.lpszMenuName = NULL;
      wc.style = CS_HREDRAW | CS_VREDRAW;

      if (!RegisterClassEx(&wc))
      {
         int nResult = GetLastError();
         MessageBox(NULL,
            L"Window class creation failed",
            L"Window Class Failed",
            MB_ICONERROR);
      }
      else
         windowclass3registeredbefore = true;
   }
   hwnd = CreateWindowEx(NULL,
      wc.lpszClassName,
      L"Window 3",
      WS_OVERLAPPEDWINDOW,
      200,
      190,
      640,
      480,
      NULL,
      NULL,
      hInst,
      NULL                /* No Window Creation data */
      );

   if (!hwnd)
   {
      int nResult = GetLastError();

      MessageBox(NULL,
         L"Window creation failed",
         L"Window Creation Failed",
         MB_ICONERROR);
   }

   ShowWindow(hwnd, nShowCmd);
}

void createwindow4(WNDCLASSEX& wc, HWND& hwnd, HINSTANCE hInst, int nShowCmd) {
   if (windowclass4registeredbefore == false) {
      ZeroMemory(&wc, sizeof(WNDCLASSEX));
      wc.cbClsExtra = NULL;
      wc.cbSize = sizeof(WNDCLASSEX);
      wc.cbWndExtra = NULL;
      wc.hbrBackground = (HBRUSH)COLOR_WINDOW;
      wc.hCursor = LoadCursor(NULL, IDC_ARROW);
      wc.hIcon = NULL;
      wc.hIconSm = NULL;
      wc.hInstance = hInst;
      wc.lpfnWndProc = (WNDPROC)windowprocessforwindow4;
      wc.lpszClassName = L"window class 4";
      wc.lpszMenuName = NULL;
      wc.style = CS_HREDRAW | CS_VREDRAW;

      if (!RegisterClassEx(&wc))
      {
         int nResult = GetLastError();
         MessageBox(NULL,
            L"Window class creation failed",
            L"Window Class Failed",
            MB_ICONERROR);
      }
      else
         windowclass4registeredbefore = true;
   }
   hwnd = CreateWindowEx(NULL,
      wc.lpszClassName,
      L"Window 4",
      WS_OVERLAPPEDWINDOW,
      200,
      210,
      640,
      480,
      NULL,
      NULL,
      hInst,
      NULL                /* No Window Creation data */
      );

   if (!hwnd)
   {
      int nResult = GetLastError();

      MessageBox(NULL,
         L"Window creation failed",
         L"Window Creation Failed",
         MB_ICONERROR);
   }

   ShowWindow(hwnd, nShowCmd);
}

// windows process functions

LRESULT CALLBACK windowprocessforwindow1(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
   switch (message) {
   case WM_CREATE:
      window1open = true;
      CreateWindowEx(NULL,
         L"BUTTON",
         L"Open Window 2",
         WS_TABSTOP | WS_VISIBLE |
         WS_CHILD | BS_DEFPUSHBUTTON,
         50,
         220,
         150,
         24,
         hwnd,
         (HMENU)createwindowbuttoninwindow1,
         GetModuleHandle(NULL),
         NULL);
      break;
   case WM_DESTROY:
      window1open = false;
      break;
   case WM_COMMAND:
      switch LOWORD(wParam) {
      case createwindowbuttoninwindow1:
         windowtoopenenum = window2;
         break;
      }
   }
   return DefWindowProc(hwnd, message, wParam, lParam);

}

LRESULT CALLBACK windowprocessforwindow2(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
   switch (message) {
   case WM_CREATE:
      window2open = true;
      CreateWindowEx(NULL,
         L"BUTTON",
         L"Open Window 3",
         WS_TABSTOP | WS_VISIBLE |
         WS_CHILD | BS_DEFPUSHBUTTON,
         50,
         220,
         150,
         24,
         hwnd,
         (HMENU)createwindowbuttoninwindow2,
         GetModuleHandle(NULL),
         NULL);
      break;
   case WM_DESTROY:
      window2open = false;
      break;
   case WM_COMMAND:
      switch LOWORD(wParam) {
      case createwindowbuttoninwindow2:
         windowtoopenenum = window3;
         break;
      }
   }
   return DefWindowProc(hwnd, message, wParam, lParam);
}

LRESULT CALLBACK windowprocessforwindow3(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
   switch (message) {
   case WM_CREATE:
      window3open = true;
      CreateWindowEx(NULL,
         L"BUTTON",
         L"Open Window 4",
         WS_TABSTOP | WS_VISIBLE |
         WS_CHILD | BS_DEFPUSHBUTTON,
         50,
         220,
         150,
         24,
         hwnd,
         (HMENU)createwindowbuttoninwindow3,
         GetModuleHandle(NULL),
         NULL);
      break;
   case WM_DESTROY:
      window3open = false;
      break;
   case WM_COMMAND:
      switch LOWORD(wParam) {
      case createwindowbuttoninwindow3:
         windowtoopenenum = window4;
         break;
      }
   }
   return DefWindowProc(hwnd, message, wParam, lParam);
}

LRESULT CALLBACK windowprocessforwindow4(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
   switch (message) {
   case WM_DESTROY:
      window4open = false;
      break;
   }
   return DefWindowProc(hwnd, message, wParam, lParam);
}