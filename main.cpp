/*----------------------------------------------------
 CPP

 main.cpp
								   Author: Carina Chao
								   Date: 2026/07/01
 ----------------------------------------------------*/
#include <SDKDDKVer.h>
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <combaseapi.h>

#include <algorithm>
#include <iomanip>
#include <sstream>

#include "config.h"
#include "application.h"
#include "direct3d.h"
#include "debug_text.h"
#include "system_timer.h"
#include "keyboard.h"


//Window情報

static constexpr char WINDOW_CLASS[] = "GameWindow";
static constexpr char TITLE[] = "Sprouts";

//WindowProcedure Prototype宣言

LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);


int APIENTRY WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE /*hPrevInstance*/, _In_ LPSTR /*lpCmdLine*/, _In_ int nCmdShow) {

	(void)CoInitializeEx(nullptr, COINIT_MULTITHREADED);
	SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);

	//Window Classの登録
	WNDCLASSEX wcex{};

	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, IDI_APPLICATION);
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = nullptr;
	wcex.lpszClassName = WINDOW_CLASS;
	wcex.hIconSm = LoadIcon(wcex.hInstance, IDI_APPLICATION);

	RegisterClassEx(&wcex);


	RECT window_rect{
		.left = 0, .top = 0, .right = SCREEN_WIDTH, .bottom = SCREEN_HEIGHT
	};

	constexpr DWORD WINDOW_STYLE = WS_OVERLAPPEDWINDOW & ~(WS_THICKFRAME | WS_MAXIMIZEBOX);

	AdjustWindowRect(&window_rect, WINDOW_STYLE, FALSE);

	const int window_width{ window_rect.right - window_rect.left };
	const int window_height{ window_rect.bottom - window_rect.top };

	//	メインモニターの画面解像度を取得する
	const int desktop_width = GetSystemMetrics(SM_CXSCREEN);
	const int desktop_height = GetSystemMetrics(SM_CYSCREEN);

	const int window_x = std::max((desktop_width - window_width) / 2, 0);
	const int window_y = std::max((desktop_height - window_height) / 2, 0);

	//Main Windowの作成

	HWND hWnd = CreateWindow(
		WINDOW_CLASS,
		TITLE,
		WINDOW_STYLE,
		window_x, window_y,
		window_width, window_height,
		nullptr,
		nullptr,
		hInstance,
		nullptr);


	//Initializing Direct3D
	if (Application_Initialize(hWnd)) 
	{
#ifdef _DEBUG // if a debug build , create a debug text object for displaying FPS and other debug information
		hal::DebugText debug_text(
			Direct3D_GetDevice(),
			Direct3D_GetContext(),
			L"assets/sixtyfour-Regular_ascii_512.png",
			SCREEN_WIDTH, SCREEN_HEIGHT);
#endif
		//Updating Window
		ShowWindow(hWnd, nCmdShow);
		UpdateWindow(hWnd);
		
		// FPS表示 からの追加: タイマーの初期化とスレッドアフィニティ設定
		SystemTimer_Initialize();
		LimitThreadAffinityToCurrentProc(); // 実行スレッドを固定してタイマーの誤差を防ぐ
		SystemTimer_Start();

		// FPS表示 からの追加: 計測用変数
		double elapsed_time = 0.0; // 1フレームの経過時間（秒）
		double time_accumulator = 0.0; // 経過時間の累積値
		int frame_counter = 0; // フレーム数の累積カウント
		double fps = 0.0; // 算出されたFPS値

		double fixed_time_accumulator = 0.0;
		constexpr double FIXED_DELTA_TIME = 1.0 / 60.0; // 60 FPS
		// Message Loop

		MSG msg{};
		
		do {
			if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
			else 
			{
				// FPS表示 からの追加: 前回のフレームからの経過時間（秒）を取得
				elapsed_time = SystemTimer_GetElapsedTime();
				if (elapsed_time > 0.1f)
				{
					elapsed_time = 0.1f;
				}

				time_accumulator += elapsed_time;
				frame_counter++;

				Application_Update((float)elapsed_time);
				// 1.0秒経過したら、その間のフレーム数からFPSを確定して累積をリセット
				if (time_accumulator >= 1.0) {
					fps = frame_counter / time_accumulator;
					frame_counter = 0;
					time_accumulator = 0.0;
				}
			
				fixed_time_accumulator += elapsed_time;
				int update_count = 0;
				//happens ever 1/60 seconds, 60 times per second
				while (fixed_time_accumulator >= FIXED_DELTA_TIME && update_count < 5)
				{
					fixed_time_accumulator -= FIXED_DELTA_TIME;
					Application_FixedUpdate();
					update_count++;
				}

				Direct3D_Begin();
				Application_Draw();
				//ゲームの処理
#ifdef _DEBUG
				debug_text.Clear();
				// FPS表示 からの追加: 計測されたFPSとフレーム経過時間(ms)をstd::stringstreamで整形
				std::stringstream ss;
				ss << "FPS: " << std::fixed << std::setprecision(2) << fps << " (" << std::fixed << std::setprecision(2) << (elapsed_time * 1000.0) << " ms)";
				debug_text.SetText(ss.str().c_str());
				debug_text.Draw();
#endif
				
				Direct3D_Present();
			}
		} while (msg.message != WM_QUIT);
	}
	Application_Finalize();
	
	return 0;
}

// Window Procedure
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {


	switch (message) {
	case WM_CLOSE:
		if (MessageBox(hWnd, "    Are you sure you want to quit?\n    Unsaved progress will be lost.", "Confirmation", MB_OKCANCEL | MB_DEFBUTTON2 | MB_ICONEXCLAMATION) == IDOK) {
			DestroyWindow(hWnd);
		}
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	
	case WM_ACTIVATEAPP:
		Keyboard_ProcessMessage(message, wParam, lParam);
		break; 
	
	case WM_KEYDOWN:
		[[fallthrough]];
	case WM_SYSKEYDOWN:
	case WM_KEYUP:
	case WM_SYSKEYUP:
		Keyboard_ProcessMessage(message, wParam, lParam);
		break;

	case WM_INPUT:
	case WM_MOUSEMOVE:
	case WM_LBUTTONDOWN:
	case WM_LBUTTONUP:
	case WM_RBUTTONDOWN:
	case WM_RBUTTONUP:
	case WM_MBUTTONDOWN:
	case WM_MBUTTONUP:
	case WM_MOUSEWHEEL:
	case WM_XBUTTONDOWN:
	case WM_XBUTTONUP:
	case WM_MOUSEHOVER:
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}