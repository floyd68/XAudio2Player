#pragma once

#include "framework.h"
#include "wndimpl.h"
#include <memory>

// #include <mfreadwrite.h>

class CAudioEngine;
struct AudioEngineDeleter
{
	void operator()(CAudioEngine* ) const;
};

class CXAudioWnd : public CWndImpl<CXAudioWnd>
{
public:
	CXAudioWnd(const wchar_t* szTitle);

	LRESULT OnCreate(WPARAM wParam, LPARAM lParam);
	LRESULT OnPaint(WPARAM wParam, LPARAM lParam);
	LRESULT OnCommand(WPARAM wParam, LPARAM lParam);
	LRESULT OnKeyDown(WPARAM wParam, LPARAM lParam);

private:
	
	std::unique_ptr<CAudioEngine, AudioEngineDeleter> m_pAudioEngine;
};

