#include "XAudioWnd.h"
#include "resource.h"
#include "error.h"
#include "AudioEngine.h"

DEFINE_WND(CXAudioWnd)

BEGIN_MESSAGE_MAP(CXAudioWnd)
	ON_MESSAGE(WM_CREATE, &CXAudioWnd::OnCreate)
	ON_MESSAGE(WM_PAINT, &CXAudioWnd::OnPaint)
	ON_MESSAGE(WM_COMMAND, &CXAudioWnd::OnCommand)
	ON_MESSAGE(WM_KEYDOWN, &CXAudioWnd::OnKeyDown)
END_MESSAGE_MAP


void AudioEngineDeleter::operator()(CAudioEngine* p) const
{
	delete p;
}

CXAudioWnd::CXAudioWnd(const wchar_t* szTitle)
	: CWndImpl(szTitle)
	, m_pAudioEngine(new CAudioEngine(100.0f))
{
}

LRESULT CXAudioWnd::OnCreate(WPARAM wParam, LPARAM lParam)
{
	return 0;
}


LRESULT CXAudioWnd::OnPaint(WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;
	
	HDC hdc = BeginPaint(m_hWnd, &ps);
	// TODO: ���⿡ hdc�� ����ϴ� �׸��� �ڵ带 �߰��մϴ�...
	EndPaint(m_hWnd, &ps);

	return 0;
}

static int nMusic = 0;

LRESULT CXAudioWnd::OnKeyDown(WPARAM wParam, LPARAM lParam)
{
	switch (wParam)
	{
	case VK_SPACE:
		{
			CVoice* pVoice = nullptr;

			switch (nMusic)
			{
			
			case 1: pVoice = m_pAudioEngine->CreateStreamVoice(L"Castle2.m4a"); break;
			case 2: pVoice = m_pAudioEngine->CreateStreamVoice(L"Castle3.m4a"); break;
			case 0:	pVoice = m_pAudioEngine->CreateStreamVoice(L"Castle1.m4a"); break;
			}
			if ( pVoice )
				pVoice->Play();
			nMusic = ++nMusic % 3;

		}
		break;
	}
	return 0;
}

// ���� ��ȭ ������ �޽��� ó�����Դϴ�.
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


LRESULT CXAudioWnd::OnCommand(WPARAM wParam, LPARAM lParam)
{
	int wmId = LOWORD(wParam);
	// �޴� ������ ���� �м��մϴ�:
	switch (wmId)
	{
	case IDM_ABOUT:
		DialogBox(s_hInstance, MAKEINTRESOURCE(IDD_ABOUTBOX), m_hWnd, About);
		break;

	case IDM_EXIT:
		DestroyWindow(m_hWnd);
		break;
	default:
		return DefWindowProc(m_hWnd, WM_COMMAND, wParam, lParam);
	}

	return 0;
}
