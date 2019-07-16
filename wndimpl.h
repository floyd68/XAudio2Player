#pragma once

#include <map>

template <class wnd>
class CWndImpl
{
public:
	static ATOM	RegisterClass(HINSTANCE hInstance, wchar_t* szWindowClass, LPCWSTR lpszMenuName, HICON hIcon, HICON hIconSm)
	{
		s_hInstance = hInstance;

		WNDCLASSEXW wcex;

		wcscpy_s(s_szWindowClass, szWindowClass);

		wcex.cbSize = sizeof(WNDCLASSEX);

		wcex.style = CS_HREDRAW | CS_VREDRAW;
		wcex.lpfnWndProc = WndProc;
		wcex.cbClsExtra = 0;
		wcex.cbWndExtra = 0;
		wcex.hInstance = hInstance;
		wcex.hIcon = hIcon;
		wcex.hIconSm = hIconSm;
		wcex.hCursor = LoadCursor(s_hInstance, IDC_ARROW);
		wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
		wcex.lpszMenuName = lpszMenuName;
		wcex.lpszClassName = s_szWindowClass;

		return RegisterClassExW(&wcex);
	}

	LRESULT OnMessage(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
	{
		auto it = s_mapMessageHandler.find(message);
		if (it != s_mapMessageHandler.end())
		{
			auto fn = it->second;
			return ((wnd*)(this)->*fn)(wParam, lParam);
		}
		else
		{
			switch (message)
			{
			case WM_DESTROY:
				PostQuitMessage(0);
				break;
			}
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
	}

public:
	CWndImpl(const wchar_t* szTitle)
	{
		s_tempWnd = this;
		m_hWnd = CreateWindowW(s_szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
			CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, s_hInstance, nullptr);
		if (m_hWnd)
			s_mapWnds.insert({ m_hWnd, this });
		s_tempWnd = nullptr;
	}
	~CWndImpl()
	{
		s_mapWnds.erase(m_hWnd);
		DestroyWindow(m_hWnd);
	}

	void ShowWindow(int nCmdShow)
	{
		::ShowWindow(m_hWnd, nCmdShow);
	}

	void UpdateWindow()
	{
		::UpdateWindow(m_hWnd);
	}

protected:
	HWND m_hWnd;
	static HINSTANCE s_hInstance;

private:
	static CWndImpl<wnd>* s_tempWnd;
	static std::map<HWND, CWndImpl*> s_mapWnds;
	static wchar_t s_szWindowClass[256];

	using MessageHandler = LRESULT(wnd::*)(WPARAM, LPARAM);
	static std::map<UINT, CWndImpl<wnd>::MessageHandler> s_mapMessageHandler;

	static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
	{
		auto it = s_mapWnds.find(hWnd);
		if (it != s_mapWnds.end())
			return it->second->OnMessage(hWnd, message, wParam, lParam);
		else
		{
			if (s_tempWnd)
				return s_tempWnd->OnMessage(hWnd, message, wParam, lParam);
			else
				return DefWindowProc(hWnd, message, wParam, lParam);
		}
	}
};


#define DEFINE_WND(x) \
	CWndImpl<x>* CWndImpl<x>::s_tempWnd = nullptr; \
	std::map<HWND, CWndImpl<x>*> CWndImpl<x>::s_mapWnds; \
	HINSTANCE CWndImpl<x>::s_hInstance = nullptr; \
	wchar_t CWndImpl<x>::s_szWindowClass[256] = { 0, };

#define BEGIN_MESSAGE_MAP(x) std::map<UINT, CWndImpl<x>::MessageHandler> CWndImpl<x>::s_mapMessageHandler = {
#define ON_MESSAGE(msg, handler) { msg, handler },
#define END_MESSAGE_MAP };