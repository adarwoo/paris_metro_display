#include "stdafx.h"
#include "winsim.h"

#include "lib/reactor.h"
#include "driver/fb.h"
#include "lib/timer.h"
#include "lib/alert.h"
#include "core/measurements.h"
#include "core/sequencer.h"
#include "lib/tz.h"

#include "calendar.h"
#include "station_points.h"

#include "logger.h"

static int g_render_index = 0;

// Non declared hook functions for simulation
extern void measurement_set_temperature(int16_t t);
extern void rtc_force_timedate(calendar_date &NewDateTime);

// From win_fb
extern int FbLeds[48];

// Threads to simulate the timer, framebuffer and reactor
HANDLE hThreads[3];

// Flag to tell the threads to bugger off
bool AppExitFlag = FALSE;

// Global pointer for static callbacks
PldSimulator* pDemoApp = nullptr;

//
// Test dates for the dialog box
//
using SelectDate = std::pair<calendar_date, std::wstring>;

// Create special date to test out the time display
// These are UTC times!!!
// Month is 0-11
SelectDate test_dates[]
{
   { { 0,  0,   0,  0,  0, 2016, 0 }, L"Start of the hour" },
   { { 40, 59, 11,  0,  0, 2016, 0 }, L"20 seconds before the hour" },
   { { 40, 29, 11,  0,  0, 2016, 0 }, L"20 seconds before the haft gong" },
   { { 40, 58, 23, 30, 11, 2016, 0 }, L"80 seconds before new year" },
};

#define COMBO_ID     201
#define EDIT_ID      202
#define SHORT_BTN_ID 203
#define LONG_BTN_ID  204
#define IDT_TIMER_REFRESH_TIME 2000

HWND hWndComboBox = NULL;
HWND hWndEditBox = NULL;
HWND hWndPushButtonShort = NULL;
HWND hWndPushButtonLong = NULL;
HWND hWndDlg = NULL;

WNDPROC Edit_proc;

LRESULT CALLBACK EditProcWithEnter(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
   switch (msg) 
   {
   case WM_CHAR:
   {
      switch (wParam) 
      {
         case VK_RETURN: 
            {
               // Grab the int
               int readout = GetDlgItemInt(hWndDlg, EDIT_ID, 0, true);

               if (readout < (INT8_MIN*10) || readout > (INT8_MAX * 10))
               {
                  SetDlgItemInt(hWndDlg, EDIT_ID, measurement_get_temperature(), true);
               }
               else
               {
                  measurement_set_temperature(readout);
               }

               // Edit loose the focus (give back to dialog)
               SetFocus(hWndDlg);

               return 0;
            }
            break;
         }
      }
      break;
   }

   return CallWindowProc(Edit_proc, hwnd, msg, wParam, lParam);
}

LRESULT CALLBACK windowprocessforwindow2(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
   static bool fEditFocus = false;
   switch (message)
   {
   case WM_CREATE:
      {
         hWndComboBox = CreateWindowEx(NULL,
            L"COMBOBOX",
            L"Date picker",
            CBS_DROPDOWNLIST | CBS_HASSTRINGS | WS_TABSTOP | WS_OVERLAPPED | WS_VISIBLE | WS_CHILD,
            10,
            10,
            280,
            100,
            hwnd,
            (HMENU)COMBO_ID,
            GetModuleHandle(NULL),
            NULL);

         for (auto item : test_dates)
         {
            // Add string to combobox.
            SendMessage(hWndComboBox, (UINT)CB_ADDSTRING, (WPARAM)0, (LPARAM)item.second.c_str());
         }

         // Send the CB_SETCURSEL message to display an initial item 
         //  in the selection field  
         SendMessage(hWndComboBox, CB_SETCURSEL, (WPARAM)-1, (LPARAM)0);

         //
         // Add the temperature edit
         //
         hWndEditBox = CreateWindowEx(
            WS_EX_CLIENTEDGE,
            L"EDIT",
            L"",
            WS_VISIBLE | WS_CHILD | WS_BORDER | ES_LEFT,
            300,
            10,
            50,
            26,
            hwnd,
            (HMENU)EDIT_ID,
            GetModuleHandle(NULL),
            NULL);

         // Grab the default temp
         SetWindowTextW(hWndEditBox, std::to_wstring(measurement_get_temperature()).c_str());

         // Subclass
         Edit_proc = (WNDPROC)GetWindowLongPtr(hWndEditBox, GWLP_WNDPROC);
         SetWindowLong(hWndEditBox, GWLP_WNDPROC, (LONG_PTR)EditProcWithEnter);

         hWndPushButtonShort = CreateWindowEx(NULL,
            L"BUTTON",
            L"Short",
            WS_TABSTOP | WS_OVERLAPPED | WS_VISIBLE | WS_CHILD,
            360,
            10,
            40,
            26,
            hwnd,
            (HMENU)SHORT_BTN_ID,
            GetModuleHandle(NULL),
            NULL);

         hWndPushButtonLong = CreateWindowEx(NULL,
            L"BUTTON",
            L"Long",
            WS_TABSTOP | WS_OVERLAPPED | WS_VISIBLE | WS_CHILD,
            410,
            10,
            40,
            26,
            hwnd,
            (HMENU)LONG_BTN_ID,
            GetModuleHandle(NULL),
            NULL);

         SetTimer(hwnd,             // handle to main window 
            IDT_TIMER_REFRESH_TIME, // timer identifier 
            1000,                   // 1-second interval 
            (TIMERPROC)NULL);       // no timer callback 
      }
      break;
   case WM_DESTROY:
      break;

   case WM_COMMAND:
   {
      switch LOWORD(wParam)
      {
      case SHORT_BTN_ID:
         sequencer_switch_short();
         break;
      case LONG_BTN_ID:
         sequencer_switch_long();
         break;
      case COMBO_ID:
         if (HIWORD(wParam) == CBN_SELCHANGE)
         {
            int ItemIndex = (int)SendMessage((HWND)lParam, (UINT)CB_GETCURSEL,
               (WPARAM)0, (LPARAM)0);
            rtc_force_timedate(test_dates[ItemIndex].first);
            return 0;
         }
         break;
      default:
         break;
      }
   }
      break;

   case WM_PAINT:
      {
         PAINTSTRUCT ps;
         HDC hdc;
         static TCHAR buf[32];
         tz_datetime_t localAdjustedTime;
         tz_now(&localAdjustedTime);
         swprintf(buf, 32, L"%.4d/%.2d/%.2d %.2d:%.2d:%.2d",
            localAdjustedTime.year, localAdjustedTime.month + 1, localAdjustedTime.date + 1,
            localAdjustedTime.hour, localAdjustedTime.minute, localAdjustedTime.second);

         // Write the date
         hdc = BeginPaint(hwnd, &ps);
         TextOut(hdc, 20, 38, buf, 20);
         EndPaint(hwnd, &ps);
         return 0L;
      }
      break;
   case WM_TIMER:
      if ( wParam == IDT_TIMER_REFRESH_TIME )
      {
         RECT area;
         GetClientRect(hwnd, &area);
         area.top = 38;
         area.left = 20;
         //{ 20, 38, 200, 20 };
         InvalidateRect(hwnd, &area, TRUE);
      }
      break;
   }

   return DefWindowProc(hwnd, message, wParam, lParam);
}

void CreateSideWindow(HWND hMain, WNDCLASSEX& wc, HWND& hwnd, HINSTANCE hInst, int nShowCmd) {
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

   RECT parentRect;
   GetWindowRect(hMain, &parentRect);

   hWndDlg = CreateWindowEx(NULL,
      wc.lpszClassName,
      L"Simulator control",
      WS_VISIBLE | WS_GROUP,
      parentRect.left,
      parentRect.bottom,
      500,
      100,
      hMain,
      NULL,
      hInst,
      NULL                /* No Window Creation data */
      );

   if (!hWndDlg)
   {
      int nResult = GetLastError();

      MessageBox(NULL,
         L"Window creation failed",
         L"Window Creation Failed",
         MB_ICONERROR);
   }

   ShowWindow(hWndDlg, nShowCmd);
}

HRESULT LoadGiFInResource(IStream *&pStream)
{
   ULONG size;
   ULONG written;
   const char* data;
   HMODULE handle = ::GetModuleHandle(NULL);
   HRSRC rc = ::FindResource(handle, MAKEINTRESOURCE(IDR_RERA), L"GIF");
   HGLOBAL rcData = ::LoadResource(handle, rc);

   size = ::SizeofResource(handle, rc);
   data = static_cast<const char*>(::LockResource(rcData));
   
   CreateStreamOnHGlobal(NULL, true, &pStream);

   return pStream->Write(data, size, &written);
}

PldSimulator::PldSimulator() :
   m_hwnd(NULL),
   m_pDirect2dFactory(NULL),
   m_pRenderTarget(NULL),
   m_pLightSlateGrayBrush(NULL),
   m_pCornflowerBlueBrush(NULL),
   m_pIWICFactory(NULL),
   m_pDecoder(NULL),
   m_pConverter(NULL),
   m_pBitmap(NULL)
{
}

PldSimulator::~PldSimulator()
{
   SafeRelease(&m_pDecoder);
   SafeRelease(&m_pIWICFactory);
   SafeRelease(&m_pDirect2dFactory);
   SafeRelease(&m_pRenderTarget);
   SafeRelease(&m_pLightSlateGrayBrush);
   SafeRelease(&m_pCornflowerBlueBrush);
}

void PldSimulator::RunMessageLoop()
{
   MSG msg;

   while (GetMessage(&msg, NULL, 0, 0))
   {
      TranslateMessage(&msg);
      DispatchMessage(&msg);
   }
}

HRESULT PldSimulator::Initialize()
{
   HRESULT hr;

   // Initialize device-indpendent resources, such
   // as the Direct2D factory.
   hr = CreateDeviceIndependentResources();

   if (SUCCEEDED(hr))
   {
      // Register the window class.
      WNDCLASSEX wcex = { sizeof(WNDCLASSEX) };
      wcex.style = CS_HREDRAW | CS_VREDRAW;
      wcex.lpfnWndProc = PldSimulator::WndProc;
      wcex.cbClsExtra = 0;
      wcex.cbWndExtra = sizeof(LONG_PTR);
      wcex.hInstance = HINST_THISCOMPONENT;
      wcex.hbrBackground = NULL;
      wcex.lpszMenuName = NULL;
      wcex.hCursor = LoadCursor(NULL, IDI_APPLICATION);
      wcex.lpszClassName = L"D2DDemoApp";

      RegisterClassEx(&wcex);

      // Because the CreateWindow function takes its size in pixels,
      // obtain the system DPI and use it to scale the window size.
      FLOAT dpiX, dpiY;

      // The factory returns the current system DPI. This is also the value it will use
      // to create its own windows.
      m_pDirect2dFactory->GetDesktopDpi(&dpiX, &dpiY);

      // Create the window.
      m_hwnd = CreateWindow(
         L"D2DDemoApp",
         L"Direct2D Demo App",
         WS_OVERLAPPEDWINDOW,
         CW_USEDEFAULT,
         CW_USEDEFAULT,
         static_cast<UINT>(ceil(1286.0 * dpiX / 96.f)),
         static_cast<UINT>(ceil(237.0 * dpiY / 96.f)),
         NULL,
         NULL,
         HINST_THISCOMPONENT,
         this
         );

      hr = m_hwnd ? S_OK : E_FAIL;

      if (SUCCEEDED(hr))
      {
         ShowWindow(m_hwnd, SW_SHOWNORMAL);
         UpdateWindow(m_hwnd);
      }

      WNDCLASSEX windowclassforwindow2;
      HWND handleforwindow2;

      CreateSideWindow(m_hwnd, windowclassforwindow2, handleforwindow2, HINST_THISCOMPONENT, SW_SHOWNORMAL);
   }

   return hr;
}

HRESULT PldSimulator::CreateDeviceIndependentResources()
{
   HRESULT hr = S_OK;

   // Create a Direct2D factory.
   hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_MULTI_THREADED, &m_pDirect2dFactory);

   if (SUCCEEDED(hr))
   {
      hr = CoCreateInstance(
         CLSID_WICImagingFactory,
         NULL,
         CLSCTX_INPROC_SERVER,
         IID_IWICImagingFactory,
         (LPVOID*)&m_pIWICFactory
         );
   }

   if (SUCCEEDED(hr))
   {
      IStream *pStream;
      
      hr = LoadGiFInResource(pStream);

      // Load from the resource file
      if (SUCCEEDED(hr)) 
      {
         hr = m_pIWICFactory->CreateDecoderFromStream(
            pStream,
            NULL,
            WICDecodeMetadataCacheOnDemand,
            &m_pDecoder);
      }
   }

   return hr;
}

HRESULT PldSimulator::CreateDeviceResources()
{
   HRESULT hr = S_OK;
   static COLORREF shades[] = {
      RGB(0, 0, 0),       // 0
      RGB(88, 102, 0),    // 4
      RGB(22, 26, 0),     // 1
      RGB(44, 51, 0),     // 2
      RGB(66, 77, 0),     // 3
      RGB(88, 102, 0),    // 4
      RGB(111, 128, 0),   // 5
      RGB(133, 153, 0),   // 6
      RGB(155, 179, 0),   // 7
      RGB(177, 204, 0),   // 8
      RGB(199, 230, 0),   // 9
      RGB(221, 255, 0),   // 10
      RGB(224, 255, 26),  // 11
      RGB(228, 255, 51),  // 12
      RGB(231, 255, 77),  // 13
      RGB(235, 255, 102), // 14
      RGB(238, 255, 128), // 15
   };

   if (!m_pRenderTarget)
   {
      RECT rc;
      GetClientRect(m_hwnd, &rc);

      D2D1_SIZE_U size = D2D1::SizeU(
         rc.right - rc.left,
         rc.bottom - rc.top
         );

      // Create a Direct2D render target.
      hr = m_pDirect2dFactory->CreateHwndRenderTarget(
         D2D1::RenderTargetProperties(),
         D2D1::HwndRenderTargetProperties(m_hwnd, size),
         &m_pRenderTarget
         );


      if (SUCCEEDED(hr))
      {
         // Create a gray brush.
         hr = m_pRenderTarget->CreateSolidColorBrush(
            D2D1::ColorF(D2D1::ColorF::LightSlateGray),
            &m_pLightSlateGrayBrush
            );
      }

      // Create the 16 shades of yellow
      ID2D1GradientStopCollection *pGradientStops = NULL;
      D2D1_GRADIENT_STOP gradientStops[2];
      gradientStops[0].color = D2D1::ColorF(D2D1::ColorF::Yellow, 1);
      gradientStops[0].position = 0.0f;
      gradientStops[1].color = D2D1::ColorF(D2D1::ColorF::DimGray, 1);
      gradientStops[1].position = 1.0f;

      // Create the ID2D1GradientStopCollection from a previously
      // declared array of D2D1_GRADIENT_STOP structs.
      hr = m_pRenderTarget->CreateGradientStopCollection(
         gradientStops,
         2,
         D2D1_GAMMA_2_2,
         D2D1_EXTEND_MODE_CLAMP,
         &pGradientStops
         );

      for (size_t i = 0; i < 16; ++i)
      {
         float g = 2.0f*(float)log(1.0f + i*0.8f)*32.0f / 4.0f;
         float s = 2.0f * (float)log(1 + i) / 6.1f + 0.5f;

         gradientStops[0].color =
            D2D1::ColorF(D2D1::ColorF(1.0f*s, 1.0f*s, 0.2f, 1.0f));

         hr = m_pRenderTarget->CreateGradientStopCollection(
            gradientStops,
            2,
            D2D1_GAMMA_2_2,
            D2D1_EXTEND_MODE_CLAMP,
            &pGradientStops
            );

         hr = m_pRenderTarget->CreateRadialGradientBrush(
            D2D1::RadialGradientBrushProperties(
               D2D1::Point2F(g, g),
               D2D1::Point2F(0, 0),
               g,
               g),
            pGradientStops,
            &m_pLedShades[i]);

      }

      if (SUCCEEDED(hr))
      {
         // Create a blue brush.
         hr = m_pRenderTarget->CreateSolidColorBrush(
            D2D1::ColorF(D2D1::ColorF::Black),
            &m_pCornflowerBlueBrush
            );
      }

      // Retrieve the first frame of the image from the decoder
      IWICBitmapFrameDecode *pFrame = NULL;

      if (SUCCEEDED(hr))
      {
         hr = m_pDecoder->GetFrame(0, &pFrame);
      }

      if (SUCCEEDED(hr))
      {
         // Convert the image format to 32bppPBGRA
         // (DXGI_FORMAT_B8G8R8A8_UNORM + D2D1_ALPHA_MODE_PREMULTIPLIED).
         hr = m_pIWICFactory->CreateFormatConverter(&m_pConverter);
      }

      if (SUCCEEDED(hr))
      {
         hr = m_pConverter->Initialize(
            pFrame,
            GUID_WICPixelFormat32bppPBGRA,
            WICBitmapDitherTypeNone,
            NULL,
            0.f,
            WICBitmapPaletteTypeMedianCut
            );
      }

      if (SUCCEEDED(hr))
      {

         // Create a Direct2D bitmap from the WIC bitmap.
         hr = m_pRenderTarget->CreateBitmapFromWicBitmap(
            m_pConverter,
            NULL,
            &m_pBitmap
            );
      }
   }

   return hr;
}

void PldSimulator::DiscardDeviceResources()
{
   SafeRelease(&m_pRenderTarget);
   SafeRelease(&m_pLightSlateGrayBrush);
   SafeRelease(&m_pCornflowerBlueBrush);
}

fb_mem_t avirer;

/*
 * Static method
 */
LRESULT CALLBACK PldSimulator::WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
   LRESULT result = 0;

   if (message == WM_CREATE)
   {
      LPCREATESTRUCT pcs = (LPCREATESTRUCT)lParam;
      PldSimulator *pDemoApp = (PldSimulator *)pcs->lpCreateParams;

      ::SetWindowLongPtrW(
         hwnd,
         GWLP_USERDATA,
         PtrToUlong(pDemoApp)
         );

      result = 1;
   }
   else
   {
      bool wasHandled = false;

      if (pDemoApp)
      {
         switch (message)
         {
         case WM_SIZE:
         {
            UINT width = LOWORD(lParam);
            UINT height = HIWORD(lParam);

            pDemoApp->OnResize(width, height);
         }
         result = 0;
         wasHandled = true;
         break;

         case WM_DISPLAYCHANGE:
         {
            InvalidateRect(hwnd, NULL, FALSE);
         }
         result = 0;
         wasHandled = true;
         break;

         case WM_PAINT:
         {
            pDemoApp->OnRender();
            ValidateRect(hwnd, NULL);
         }
         result = 0;
         wasHandled = true;
         break;

         case WM_CLOSE:
            AppExitFlag = true;
            WaitForMultipleObjects(3, hThreads, TRUE, INFINITE);
            result = 1;
            break;

         case WM_DESTROY:
         {
            PostQuitMessage(0);
         }
         result = 1;
         wasHandled = true;
         break;
         }
      }

      if (!wasHandled)
      {
         result = DefWindowProc(hwnd, message, wParam, lParam);
      }
   }

   return result;
}

HRESULT PldSimulator::OnRender()
{
   HRESULT hr = S_OK;

   hr = CreateDeviceResources();

   if (SUCCEEDED(hr))
   {
      m_pRenderTarget->BeginDraw();

      m_pRenderTarget->SetTransform(D2D1::Matrix3x2F::Identity());

      m_pRenderTarget->Clear(D2D1::ColorF(D2D1::ColorF::White));
      D2D1_SIZE_F rtSize = m_pRenderTarget->GetSize();

      // Retrieve the size of the bitmap.
      D2D1_SIZE_F size = m_pBitmap->GetSize();

      D2D1_POINT_2F upperLeftCorner = D2D1::Point2F(0.f, 0.f);

      // Draw a bitmap.
      m_pRenderTarget->DrawBitmap(
         m_pBitmap,
         D2D1::RectF(
            upperLeftCorner.x,
            upperLeftCorner.y,
            upperLeftCorner.x + rtSize.width, //scaledWidth,
            upperLeftCorner.y + rtSize.height) // scaledHeight)
         );

      hr = DrawLeds(false);
   }

   if (SUCCEEDED(hr))
   {
      hr = m_pRenderTarget->EndDraw();
   }

   if (hr == D2DERR_RECREATE_TARGET)
   {
      hr = S_OK;
      DiscardDeviceResources();
   }

   return hr;
}

HRESULT PldSimulator::DrawLeds(bool draw)
{
   HRESULT hr = S_OK;

   D2D1_SIZE_F rtSize = m_pRenderTarget->GetSize();

   if (draw) m_pRenderTarget->BeginDraw();

   const double xfact = rtSize.width / 2560.0 * 2.02;
   const double yfact = rtSize.height / 396.0 * 2.0;

   int index = -1;

   for (auto pt : g_station_points)
   {
      if (++index < 2)
         continue;

      D2D1_POINT_2F led = D2D1::Point2F(
         0.0f + pt.x * (float)xfact,
         0.0f + pt.y * (float)yfact
         );

      D2D1_ELLIPSE ellipse = D2D1::Ellipse(led, 3.44f*(float)xfact, 3.44f*(float)yfact);
      m_pLedShades[FbLeds[index]]->SetCenter(led);

      m_pRenderTarget->DrawEllipse(ellipse, m_pCornflowerBlueBrush, 0.0f);
      m_pRenderTarget->FillEllipse(ellipse, m_pLedShades[FbLeds[index]]);
   }

   if (draw) hr = m_pRenderTarget->EndDraw();

   return hr;
}


void PldSimulator::OnResize(UINT width, UINT height)
{
   if (m_pRenderTarget)
   {
      // Note: This method can fail, but it's okay to ignore the
      // error here, because the error will be returned again
      // the next time EndDraw is called.
      m_pRenderTarget->Resize(D2D1::SizeU(width, height));
   }
}

extern DWORD WINAPI FbTask(LPVOID lpParam);
extern DWORD WINAPI ReactorTask(LPVOID lpParam);
extern DWORD WINAPI TimerTask(LPVOID);

int WINAPI WinMain(
   HINSTANCE /* hInstance */,
   HINSTANCE /* hPrevInstance */,
   LPSTR /* lpCmdLine */,
   int /* nCmdShow */
   )
{
   // Use HeapSetInformation to specify that the process should
   // terminate if the heap manager detects an error in any heap used
   // by the process.
   // The return value is ignored, because we want to continue running in the
   // unlikely event that HeapSetInformation fails.
   HeapSetInformation(NULL, HeapEnableTerminationOnCorruption, NULL, 0);

   if (SUCCEEDED(CoInitialize(NULL)))
   {
      {
         PldSimulator app;
         pDemoApp = &app;

         if (SUCCEEDED(app.Initialize()))
         {
            SetEnvironmentVariable(L"LOG", L"info");
            LOG_INIT();
            LOG_MILE("main", "Initialising APIs");

            // Initialise the reactor thread
            alert_init();       // Allow alerts
            reactor_init();     // Prepare the reactor
            timer_init();       // Ready the timer API
            fb_init();          // Ready the frame buffer API
            measurement_init(); // Ready the systems measurements (lum and temp)
            sequencer_start();

            // Create a thread for the simulated framebuffer
            hThreads[0] = CreateThread(0, 0, FbTask, (LPVOID)&app, 0, 0);

            // Create a thread for the simulated reactor
            hThreads[1] = CreateThread(0, 0, ReactorTask, (LPVOID)&app, 0, 0);

            // Create a thread for the simulated reactor
            hThreads[1] = CreateThread(0, 0, TimerTask, (LPVOID)&app, 0, 0);

            app.RunMessageLoop();
         }
      }

      CoUninitialize();
   }

   return 0;
}
