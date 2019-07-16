// XAudio2Player.cpp : 애플리케이션에 대한 진입점을 정의합니다.
//

#include "framework.h"
#include "XAudio2Player.h"
#include "XAudioWnd.h"
#include <combaseapi.h>

const int MAX_LOADSTRING = 100;

// 전역 변수:
HINSTANCE hInst;                                // 현재 인스턴스입니다.
WCHAR szTitle[MAX_LOADSTRING];                  // 제목 표시줄 텍스트입니다.
WCHAR szWindowClass[MAX_LOADSTRING];
CXAudioWnd* g_pWnd = nullptr;

// 이 코드 모듈에 포함된 함수의 선언을 전달합니다:
BOOL                InitInstance(HINSTANCE, int);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

	HRESULT hr = CoInitializeEx(0, COINIT_MULTITHREADED);
	if (FAILED(hr))
		return -1;

    // 전역 문자열을 초기화합니다.
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_XAUDIO2PLAYER, szWindowClass, MAX_LOADSTRING);

	
	LPCWSTR lpszMenuName = MAKEINTRESOURCEW(IDC_XAUDIO2PLAYER);
	HICON hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_XAUDIO2PLAYER));
	HICON hIconSm = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_SMALL));

    CXAudioWnd::RegisterClass(hInstance, szWindowClass, lpszMenuName, hIcon, hIconSm);

    // 애플리케이션 초기화를 수행합니다:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_XAUDIO2PLAYER));

    MSG msg;

    // 기본 메시지 루프입니다:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }
	if (g_pWnd)
		delete g_pWnd;

	CoUninitialize();

    return (int) msg.wParam;
}



//
//   함수: InitInstance(HINSTANCE, int)
//
//   용도: 인스턴스 핸들을 저장하고 주 창을 만듭니다.
//
//   주석:
//
//        이 함수를 통해 인스턴스 핸들을 전역 변수에 저장하고
//        주 프로그램 창을 만든 다음 표시합니다.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // 인스턴스 핸들을 전역 변수에 저장합니다.

   g_pWnd = new CXAudioWnd(szTitle);
   g_pWnd->ShowWindow(nCmdShow);
   g_pWnd->UpdateWindow();

   return TRUE;
}
